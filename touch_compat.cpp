#include <android-base/file.h>
#include <utils/String16.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Android 16 added String16 move assignment without changing String16's data
// layout. The stock Infiniti touch HAL references that symbol, while PBRP's
// Android 12.1 libutils has the ABI-compatible copy operation only. A copy is
// a valid move implementation and keeps the moved-from object valid.
extern "C" __attribute__((visibility("default"))) android::String16*
pbrp_string16_move_assign(android::String16* destination,
                          android::String16* source)
        __asm__("_ZN7android8String16aSEOS0_");

extern "C" android::String16* pbrp_string16_move_assign(
        android::String16* destination, android::String16* source) {
    if (destination != source) {
        destination->setTo(*source);
    }
    return destination;
}

// Instantiation used by the Android 16 Qualcomm boot-control library.  The
// older libbase has Trim for std::string but does not export the string_view
// forwarding-template symbol.
__attribute__((visibility("default"))) std::string
pbrp_trim_string_view(std::string_view&& input)
        __asm__("_ZN7android4base4TrimINSt3__117basic_string_viewIcNS2_11char_traitsIcEEEEEENS2_12basic_stringIcS5_NS2_9allocatorIcEEEEOT_");

std::string pbrp_trim_string_view(std::string_view&& input) {
    static constexpr std::string_view kWhitespace = " \t\n\r\f\v";
    const std::size_t first = input.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos) return {};
    const std::size_t last = input.find_last_not_of(kWhitespace);
    return std::string(input.substr(first, last - first + 1));
}

// Android 16-generated stable AIDL libraries register an optional transaction
// name table for debugging. Android 12.1's Binder NDK predates that API, but
// omitting the table has no effect on Binder transactions or wire compatibility.
extern "C" __attribute__((visibility("default"))) void
AIBinder_Class_setTransactionCodeToFunctionNameMap(
        void* /* clazz */, const char** /* transaction_code_to_function */, std::size_t /* length */) {}

// API 33 added an optional post-unlink cleanup callback for Binder death
// recipients. The recovery OMAPI daemon is short-lived and keeps its death
// recipient for the process lifetime, so the Android 12.1 behavior is safe.
extern "C" __attribute__((visibility("default"))) void
AIBinder_DeathRecipient_setOnUnlinked(void* /* recipient */, void (* /* on_unlinked */)(void*)) {}

// Newer Qualcomm/NXP vendor services use android::base::HexString for logging
// and diagnostic identifiers. Reproduce the modern libbase helper without
// replacing PBRP's older libbase globally.
namespace android::base {

__attribute__((visibility("default"))) std::string HexString(const void* bytes, std::size_t length) {
    static constexpr char kHexDigits[] = "0123456789abcdef";
    const auto* input = static_cast<const std::uint8_t*>(bytes);
    std::string output(length * 2, '\0');

    for (std::size_t index = 0; index < length; ++index) {
        output[index * 2] = kHexDigits[input[index] >> 4];
        output[index * 2 + 1] = kHexDigits[input[index] & 0x0f];
    }
    return output;
}

// Tokenize was promoted to a public libbase helper after Android 12.1.  Split
// on any delimiter character and collapse delimiter runs, matching the modern
// implementation used by Qualcomm's boot-control library.
__attribute__((visibility("default"))) std::vector<std::string> Tokenize(
        const std::string& input, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::size_t start = input.find_first_not_of(delimiters);
    while (start != std::string::npos) {
        const std::size_t end = input.find_first_of(delimiters, start);
        tokens.emplace_back(input.substr(start, end - start));
        if (end == std::string::npos) break;
        start = input.find_first_not_of(delimiters, end);
    }
    return tokens;
}

// Newer libbase accepts string_view here. Delegate to the Android 12.1 string
// overload so path edge cases retain the platform implementation's behavior.
__attribute__((visibility("default"))) std::string Basename(std::string_view path) {
    return Basename(std::string(path));
}

}  // namespace android::base
