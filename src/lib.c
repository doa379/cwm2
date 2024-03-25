#include <stdlib.h>
#include <string.h>
#include <lib.h>

blk_t init_blk(const size_t UNITSIZE, const size_t RESERVE) {
  void* blk = { malloc((RESERVE + 1) * UNITSIZE) };
  return (blk_t) {
    .blk = blk,
    .end = blk,
    .reserve = RESERVE, 
    .unit = UNITSIZE, 
    .size = 0
  };
}

void deinit_blk(blk_t* blk) {
  free(blk->blk);
}

void* beg(blk_t* blk) {
  return blk->blk;
}

void* end(blk_t* blk) {
  return (char*) blk->blk + blk->size * blk->unit;
}

void* map_dev(blk_t* blk, const void* DEV) {
  // Return address of dev
  if (blk->size < blk->reserve) {
    memcpy((char*) blk->blk + blk->size * blk->unit, DEV, blk->unit);
    blk->size++;
    blk->end = end(blk);
    return (char*) blk->end - blk->unit;
  } else {
    void* ptr = {
      realloc(blk->blk, (blk->size + blk->reserve + 1) * blk->unit) };
    if (ptr) {
      blk->blk = ptr;
      blk->reserve += blk->size;
      memcpy((char*) blk->blk + blk->size * blk->unit, DEV, blk->unit);
      blk->size++;
      blk->end = end(blk);
      return (char*) blk->end - blk->unit;
    }
  }

  return NULL;
}

void unmap_dev(blk_t* blk, void* dev) {
  const char* P = { (char*) dev + blk->unit };
  memcpy(dev, P, (char*) blk->end - P);
  blk->size--;
  blk->end = end(blk);
}

void* find_dev(blk_t* blk, const void* DEV) {
  char* p;
  for (p = beg(blk); p != (char*) blk->end && p != DEV; p += blk->unit);
  return p != (char*) blk->end ? p : NULL;
}

void* prev_dev(blk_t* blk, void* dev) {
  return blk->size == 0 ? NULL :
    blk->size == 1 ? dev :
    dev == beg(blk) ? (char*) end(blk) - blk->unit : 
    (char*) dev - blk->unit;
}

void* next_dev(blk_t* blk, void* dev) {
  return blk->size == 0 ? NULL :
    blk->size == 1 ? dev :
    dev == (char*) end(blk) - blk->unit ? beg(blk) :
    (char*) dev + blk->unit;
}

size_t dist(blk_t* blk, const void* DEV) {
  return ((char*) DEV - (char*) blk->blk) / blk->unit;
}
