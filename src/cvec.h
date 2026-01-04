#pragma once

#include <stddef.h>

typedef struct {
  void* beg;
  void* end;
  size_t unit;
  size_t size;
  size_t res;
} cvec_t;

cvec_t cvec_init(const size_t, const size_t);
void cvec_deinit(cvec_t*);
void* cvec_beg(cvec_t const*);
void* cvec_end(cvec_t const*);
void* cvec_back(cvec_t const*);
void* cvec_itr(cvec_t const*, size_t const);
void* cvec_map(cvec_t*, const void*);
void cvec_unmap(cvec_t*, void*);
void const* cvec_find(cvec_t*, const void*);
void* cvec_prev(cvec_t*, void*);
void* cvec_next(cvec_t*, void*);
size_t const cvec_dist(cvec_t const*, const void*);
void cvec_clear(cvec_t*);
