#pragma once

#include <stddef.h>
#include <unistd.h>

typedef struct {
  void* beg;
  void* end;
  size_t unit;
  size_t size;
  size_t res;
  unsigned char* val;
} cblk_t;

cblk_t cblk_init(const size_t, const size_t);
void cblk_deinit(cblk_t*);
void* cblk_beg(cblk_t const*);
void* cblk_end(cblk_t const*);
void* cblk_back(cblk_t const*);
void* cblk_itr(cblk_t const*, ssize_t const);
void* cblk_map(cblk_t*, const void*);
void cblk_unmap(cblk_t*, void*);
void* cblk_find(cblk_t*, const void*);
void* cblk_prev(cblk_t*, void*);
void* cblk_next(cblk_t*, void*);
ssize_t const cblk_dist(cblk_t const*, const void*);
void cblk_clear(cblk_t*);
