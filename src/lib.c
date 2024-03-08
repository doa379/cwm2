#include <stdlib.h>
#include <string.h>
#include <lib.h>

blk_t init_blk(const size_t UNITSIZE, const size_t RESERVE) {
  void* blk = malloc((RESERVE + 1) * UNITSIZE);
  return (blk_t) { blk, RESERVE, UNITSIZE, 0 };
}

void deinit_blk(blk_t* blk) {
  free(blk->blk);
  *blk = (blk_t) { 0 };
}

void* map_dev(blk_t* blk, const void* DEV) {
  // Return address of dev
  if (blk->size < blk->reserve) {
    memcpy((char*) blk->blk + blk->size * blk->unit, DEV, blk->unit);
    blk->size++;
    return (char*) blk->blk + (blk->size - 1) * blk->unit;
  } else {
    void* ptr = realloc(blk->blk, (blk->size + blk->reserve + 1) * blk->unit);
    if (ptr) {
      blk->blk = ptr;
      blk->reserve += blk->size;
      memcpy((char*) blk->blk + blk->size * blk->unit, DEV, blk->unit);
      blk->size++;
      return (char*) blk->blk + (blk->size - 1) * blk->unit;
    }
  }

  return NULL;
}

void unmap_dev(blk_t* blk, const void* DEV) {
  // Patt: move post segment
  const char* E = (char*) blk->blk + (blk->size - 1) * blk->unit;
  char* p = blk->blk;
  for (; p < E && p != DEV; p += blk->unit);
  if (p < E - blk->unit)
    memcpy(p, p + blk->unit, E - p - blk->unit);
  blk->size--;
}

void* find_dev(blk_t* blk, void* dev) {
  const char* E = (char*) blk->blk + blk->size * blk->unit;
  char* p = blk->blk;
  for (; p < E && p != dev; p += blk->unit);
  return p < E - blk->unit ? p : NULL;
}

void* prev_dev(blk_t* blk, void* dev) {
  return blk->size == 0 ? NULL :
    blk->size == 1 ? dev :
    dev == blk->blk ? (char*) blk->blk + (blk->size  - 1) * blk->unit : 
    (char*) dev - blk->unit;
}

void* next_dev(blk_t* blk, void* dev) {
  return blk->size == 0 ? NULL :
    blk->size == 1 ? dev :
    dev == (char*) blk->blk + (blk->size - 1) * blk->unit ? blk->blk :
    (char*) dev + blk->unit;
}

size_t dist(blk_t* blk, const void* DEV) {
  return ((char*) DEV - (char*) blk->blk) / blk->unit;
}
