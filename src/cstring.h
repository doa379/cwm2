#pragma once

typedef struct {
  char blk[120];
  size_t length;
} cstring_t;

void cstring_init(cstring_t*, char const*);
void cstring_app(cstring_t*, char const*);
char* cstring_beg(cstring_t*);
char* cstring_end(cstring_t const*);
size_t cstring_len(cstring_t const*);
