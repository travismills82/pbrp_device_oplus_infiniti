#!/usr/bin/env bash
#
# Apply the OnePlus 15 compatibility patches to a PitchBlack 12.1 checkout.
#
# Usage:
#   device/oplus/infiniti/scripts/apply-recovery-patches.sh
#   device/oplus/infiniti/scripts/apply-recovery-patches.sh /path/to/pbrp

set -euo pipefail

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
DEVICE_DIR="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
DEFAULT_SOURCE_ROOT="$(CDPATH= cd -- "$DEVICE_DIR/../../.." && pwd)"
SOURCE_ROOT="${1:-$DEFAULT_SOURCE_ROOT}"

fail() {
    echo "ERROR: $*" >&2
    exit 1
}

command -v git >/dev/null 2>&1 || fail "git is not installed"

apply_series() {
    local project="$1"
    local patch_group="$2"
    local expected_head="$3"
    local source_dir="$SOURCE_ROOT/$project"
    local patch_dir="$DEVICE_DIR/patches/$patch_group"
    local current_head
    local patch
    local name
    local -a series=()

    [ -d "$source_dir/.git" ] || fail "source project was not found: $source_dir"
    [ -d "$patch_dir" ] || fail "patch directory was not found: $patch_dir"

    mapfile -t series < <(find "$patch_dir" -maxdepth 1 -type f -name '*.patch' | sort)
    [ "${#series[@]}" -gt 0 ] || fail "no patches found for $project"

    current_head="$(git -C "$source_dir" rev-parse HEAD)"
    echo "$project"
    echo "  current:   $current_head"
    echo "  reference: $expected_head"

    for patch in "${series[@]}"; do
        name="$(basename "$patch")"

        git -C "$source_dir" apply --numstat "$patch" >/dev/null ||
            fail "patch syntax validation failed for $project: $name"

        if git -C "$source_dir" apply --reverse --check "$patch" >/dev/null 2>&1; then
            echo "  [already applied] $name"
            continue
        fi

        if ! git -C "$source_dir" apply --check "$patch"; then
            echo "The source may have moved beyond the revision targeted by this patch." >&2
            fail "patch applicability validation failed for $project: $name"
        fi

        git -C "$source_dir" apply "$patch"
        echo "  [applied] $name"
    done
}

apply_series \
    "bootable/recovery" \
    "bootable-recovery" \
    "c3ee88af57d32a842b2a8ff861a72a7bf24da846"
apply_series \
    "hardware/interfaces" \
    "hardware-interfaces" \
    "ae469cee0dce6d71489588126a15da8e67a50102"
apply_series \
    "system/vold" \
    "system-vold" \
    "3a05a189e3a5e77fe21eb809d343816aa1177591"
apply_series \
    "vendor/pb" \
    "vendor-pb" \
    "31bf0d4cd64bd125a8e5a2e3e54367b9344a515a"

grep -Fq 'TW_USE_AIDL_BOOT_CONTROL' \
    "$SOURCE_ROOT/bootable/recovery/partitionmanager.cpp" ||
    fail "recovery is missing AIDL boot-control routing"
grep -Fq '/system/etc/magisk/Magisk.zip' \
    "$SOURCE_ROOT/bootable/recovery/gui/theme/portrait_hdpi/extras.xml" ||
    fail "recovery is missing the bundled Magisk action"
grep -Fq 'PB_TORCH_PATH_2' \
    "$SOURCE_ROOT/bootable/recovery/gui/action.cpp" ||
    fail "recovery is missing secondary torch support"
grep -Fq 'name: "android.hardware.gatekeeper.pbrp"' \
    "$SOURCE_ROOT/hardware/interfaces/gatekeeper/aidl/Android.bp" ||
    fail "hardware/interfaces is missing the recovery Gatekeeper AIDL contract"
grep -Fq 'AidlIGatekeeper' "$SOURCE_ROOT/system/vold/Decrypt.cpp" ||
    fail "vold is missing AIDL Gatekeeper support"
grep -Fq 'mAidlDevice' "$SOURCE_ROOT/system/vold/Weaver1.cpp" ||
    fail "vold is missing AIDL Weaver support"
grep -Fq 'PB_TORCH_PATH_2' "$SOURCE_ROOT/vendor/pb/config/BoardConfigSoong.mk" ||
    fail "vendor/pb is not exporting the secondary torch path"

echo "PitchBlack Infiniti patch stack is ready."
