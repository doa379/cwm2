#include <string.h>

#include "clib.h"
#include "cstring.h"

static size_t const SIZE = 
  sizeof ((cstring_t*)0)->blk / sizeof ((cstring_t*)0)->blk[0];

void cstring_init(cstring_t* cstring, char const* string) {
  size_t const len = clib_min(strlen(string), SIZE);
  strncpy(cstring->blk, string, len);
  cstring->length = len;
}

void cstring_app(cstring_t* cstring, char const* string) {
  size_t const len = clib_min(strlen(string), SIZE - cstring->length);
  strncpy(cstring->blk + cstring->length, string, len);
  cstring->length += len;
}

char* cstring_beg(cstring_t* cstring) {
  return cstring->blk;
}

char* cstring_end(cstring_t const* cstring) {
  return (char*) cstring->blk + cstring->length;
}

size_t cblk_len(cstring_t const* cstring) {
  return cstring->length;
}
