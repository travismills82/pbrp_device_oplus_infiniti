/*
 * Copyright (C) 2022-2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <cstring>
#include <string>
#include <unordered_map>

using android::base::GetProperty;

const std::unordered_map<int, std::string> kRegionSuffixMap = {
    {27,    "IN"},
    {55,    "RU"},
    {68,    "EEA"},
    {151,   ""},    // CN
    {161,   "NA"},
    {0,     ""},    // Default
};

struct ModelInfo {
    const char* brand;              // ro.product.brand
    const char* device;             // ro.product.device
    const char* manufacturer;       // ro.product.manufacturer
    const char* model;              // ro.product.model
    const char* base_name;          // ro.product.name  w/o region suffix
    const char* supportSpr;         // vendor.display.enable_spr
};

const std::unordered_map<int, ModelInfo> kModelInfoMap = {
    {24831, {"OnePlus", "OP60FFL1", "OnePlus", "PLK110",  "PLK110",  "1"}}, // infiniti CN
    {24863, {"OnePlus", "OP60FFL1", "OnePlus", "CPH2747",  "CPH2747",  "1"}}, // infiniti EU/GLO/IN/NA
    {0,     {"OPLUS",   "SM88XX",   "OPLUS",   "SM88XX",  "SM88XX",  "0"}}, // Default
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void SetupModelProperties(const ModelInfo& info, const std::string& region) {
    std::string name = info.base_name + region;
    struct PropPair {
        const char* key;
        const char* value;
    } props[] = {
        {"ro.product.brand",            info.brand},
        {"ro.product.device",           info.device},
        {"ro.product.manufacturer",     info.manufacturer},
        {"ro.product.model",            info.model},
        {"ro.product.name",             name.c_str()},
        {"vendor.display.enable_spr",   info.supportSpr},
        {"ro.build.date.utc",           "0"},
    };
    for (const auto& p : props) {
        OverrideProperty(p.key, p.value);
    }
}

void vendor_load_properties() {
    int region = 0;
    android::base::ParseInt(GetProperty("ro.boot.oplus_region", "0"), &region);
    const auto region_iter = kRegionSuffixMap.find(region);
    const std::string& region_suffix = region_iter != kRegionSuffixMap.end()
            ? region_iter->second
            : kRegionSuffixMap.find(0)->second;

    int prjname = 0;
    android::base::ParseInt(GetProperty("ro.boot.prjname", "0"), &prjname);
    const auto model_iter = kModelInfoMap.find(prjname);
    const ModelInfo& model_info = model_iter != kModelInfoMap.end()
            ? model_iter->second
            : kModelInfoMap.find(0)->second;

    SetupModelProperties(model_info, region_suffix);
}
