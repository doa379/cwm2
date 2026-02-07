#include <stdlib.h>
#include <string.h>

#include "cstr.h"

/* Short string reserve */
static size_t const SSR = sizeof ((cstr_t*) 0)->s;

cstr_t
cstr_init(char const* str) {
  if (str == NULL) {
    return (cstr_t) {
      .len = 0,
      .res = SSR,
      .s = { 0 }
    };
  }

  cstr_t cstr = { 0 };
  size_t const l = strlen(str);
  if (l < SSR) {
    memcpy(cstr.s, str, l);
    cstr.s[l] = '\0';
    cstr.len = l;
    cstr.res = SSR;
  } else {
    char* buf = malloc(l + 1);
    if (buf) {
      memcpy(buf, str, l);
      buf[l] = '\0';
      cstr.p = buf;
      cstr.len = l;
      cstr.res = l + 1;
    }
  }

  return cstr;
}

void
cstr_app(cstr_t* const cstr, char const* str) {
  if (str == NULL) {
    return;
  }

  size_t const l = strlen(str);
  size_t const nextl = cstr->len + l;
  if (nextl < SSR && cstr->res <= SSR) {
    memcpy(cstr->s + cstr->len, str, l);
    cstr->s[nextl] = '\0';
    cstr->len = nextl;
  } else if (cstr->res <= SSR) {
    char* buf = malloc(nextl + 1);
    if (buf) {
      memcpy(buf, cstr->s, cstr->len);
      memcpy(buf + cstr->len, str, l);
      buf[nextl] = '\0';
      cstr->p = buf;
      cstr->len = nextl;
      cstr->res = nextl + 1;
    }
  } else {
    char* buf = realloc(cstr->p, nextl + 1);
    if (buf) {
      memcpy(buf + cstr->len, str, l);
      buf[nextl] = '\0';
      cstr->p = buf;
      cstr->len = nextl;
      cstr->res = nextl + 1;
    }
  }
}

void
cstr_move(cstr_t* const dst, cstr_t* const src) {
  if (dst == src) {
    return;
  }

  if (dst->res > SSR) {
    free(dst->p);
  }

  if (src->res > SSR) {
    *dst = *src;
  } else {
    memcpy(dst->s, src->s, src->len + 1);
    dst->len = src->len;
    dst->res = SSR;
  }

  src->s[0] = '\0';
  src->len = 0;
  src->res = SSR;
}

void
cstr_deinit(cstr_t* const cstr) {
  if (cstr->res > SSR) {
    free(cstr->p);
  }
}

char const*
cstr_data(cstr_t const* cstr) {
  return cstr && cstr->res > SSR ? cstr->p : 
    cstr ? cstr->s : NULL;
}

size_t
cstr_len(cstr_t const* cstr) {
  return cstr ? cstr->len : 0;
}
