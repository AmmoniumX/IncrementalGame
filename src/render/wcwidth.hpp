#pragma once

#include <uchar.h>

int mk_wcwidth(char32_t ucs);
int mk_wcswidth(const char32_t *pwcs, size_t n);
int mk_w16cswidth(const char16_t *pwcs, size_t n);
