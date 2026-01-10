#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
  void* blk;
  void* front;
  void* back;
  size_t unit;
  size_t size;
  size_t res;
  uint64_t* val;
  uint64_t* p;
} cblk_t;

cblk_t cblk_init(const size_t, const size_t);
void cblk_deinit(cblk_t*);
void* cblk_front(cblk_t const*);
void* cblk_back(cblk_t const*);
void* cblk_itr(cblk_t const*, size_t const);
void* cblk_map(cblk_t* const, const void*);
void cblk_unmap(cblk_t*, void*);
void* cblk_prev(cblk_t const*, void*);
void* cblk_next(cblk_t const*, void*);
size_t const cblk_dist(cblk_t const*, const void*);
void cblk_clear(cblk_t*);
void* cblk_val_find(cblk_t const*, void*);
