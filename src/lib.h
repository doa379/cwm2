#pragma once

typedef struct {
  void* blk;
  void* end;
  size_t reserve;
  size_t unit;
  size_t size;
  long PAD[3];
} blk_t;

blk_t init_blk(const size_t, const size_t);
void deinit_blk(blk_t*);
void* beg(blk_t*);
void* end(blk_t*);
void* itr(blk_t*, const size_t);
void* map_dev(blk_t*, const void*);
void unmap_dev(blk_t*, void*);
void* find_dev(blk_t*, const void*);
void* prev_dev(blk_t*, void*);
void* next_dev(blk_t*, void*);
size_t dist(blk_t*, const void*);
int rng(const unsigned, const unsigned);
