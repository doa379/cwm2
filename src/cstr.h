#pragma once

#include <stddef.h>

typedef struct {
  size_t len;
  size_t res;
  union {
    char s[248];
    char* p;
  };
} cstr_t;


cstr_t cstr_init(char const*);
void cstr_app(cstr_t* const, char const*);
void cstr_move(cstr_t* const, cstr_t* const);
void cstr_deinit(cstr_t* const);
char const* cstr_data(cstr_t const*);
size_t cstr_len(cstr_t const*);
