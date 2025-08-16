#pragma once

#include <wchar.h>
#include <uchar.h>
#include <string>
#include <ranges>

int mk_uswidth(const std::u32string_view u32s);
int mk_uswidth(const std::u16string_view u16s);
#if defined(_WIN32)
// On Windows, wchar_t is always 16 bits
using ustring = std::u16string;
#define WCHAR_EQUIVALENT char16_t
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
// On most Unix-like systems, wchar_t is 32 bits
using ustring = std::u32string;
#define WCHAR_EQUIVALENT char32_t
#else
#error "Unsupported platform for wchar_t width detection."
#endif

// Sanity check
static_assert(sizeof(wchar_t) == sizeof(WCHAR_EQUIVALENT), "Nonmatching WCHAR_EQUIVALENT");

inline ustring ustring_from_wstring(const std::wstring_view ws) {
    return ustring(std::from_range, ws);
}
inline int mk_wswidth(const std::wstring_view ws) {
    ustring us = ustring_from_wstring(ws);
    return mk_uswidth(us);
}

