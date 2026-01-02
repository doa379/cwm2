#pragma once

#include <wchar.h>

wchar_t const* prop_root(void);
wchar_t const* prop_name(Window const);
wchar_t const* prop_ico(Window const);
int prop_sendmsg(Window const, Atom const);
