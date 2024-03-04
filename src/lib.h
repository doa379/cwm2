#pragma once

typedef struct {
  void* blk;
  size_t reserve;
  size_t unit;
  size_t size;
} blk_t;

blk_t init_blk(const size_t, const size_t);
void deinit_blk(blk_t*);
void* map_dev(blk_t*, const void*);
void unmap_dev(blk_t*, const void*);
void* find_dev(blk_t*, void*);
void* prev_dev(blk_t*, void*);
void* next_dev(blk_t*, void*);
