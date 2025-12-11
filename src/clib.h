#pragma once

#include <unistd.h>

ssize_t clib_min(ssize_t const, ssize_t const);
ssize_t clib_max(ssize_t const, ssize_t const);
int clib_rng(const unsigned, const unsigned);
