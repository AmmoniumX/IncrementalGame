#pragma once

#include <wchar.h>
#include <uchar.h>
#include <string>
#include <ranges>

int mk_uswidth(const std::u32string_view u32s);
int mk_uswidth(const std::u16string_view u16s);

// A sanity check to ensure the size matches an expected type
static_assert(sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t),
              "Unsupported wchar_t width, expecting 16 or 32 bits");

// Determine the underlying type for ustring at compile-time
using ustring = std::conditional_t<sizeof(wchar_t) == sizeof(char16_t),
                                   std::u16string,
                                   std::u32string>;

// Final sanity check
static_assert(sizeof(wchar_t) == sizeof(ustring::value_type), "Invalid wchar_t deduction");

inline ustring ustring_from_wstring(const std::wstring_view ws) {
    return ustring(std::from_range, ws);
}
inline int mk_wswidth(const std::wstring_view ws) {
    ustring us = ustring_from_wstring(ws);
    return mk_uswidth(us);
}

