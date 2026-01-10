#include <stdlib.h>
#include <string.h>

#include "cblk.h"

cblk_t
cblk_init(size_t const unit, size_t const res) {
  void* blk = malloc(unit * res);
  uint64_t* val = calloc(res, sizeof *val);
  return (cblk_t) {
    .blk = blk,
    .front = NULL,
    .back = NULL,
    .unit = unit,
    .size = 0,
    .res = res,
    .val = val,
  };
}

void
cblk_deinit(cblk_t* cblk) {
  free(cblk->val);
  free(cblk->blk);
}

void*
cblk_front(cblk_t const* cblk) {
  for (size_t i = 0; i < cblk->res; i++)
    if (cblk->val[i])
      return (char*) cblk->blk + i * cblk->unit;

  return NULL;
}

void*
cblk_back(cblk_t const* cblk) {
  return cblk_prev(cblk, cblk->front);
}

void*
cblk_itr(cblk_t const* cblk, size_t const n) {
  void* dev = cblk->front;
  if (dev == NULL)
    return NULL;

  for (size_t i = 0; i < n; i++)
    dev = cblk_next(cblk, dev);

  return dev;
}

void*
cblk_map(cblk_t* const cblk, void const* dev) {
  /* Returns address of mapping */
  for (size_t i = 0; i < cblk->res; i++) {
    if (cblk->val[i] == 0) {
      void* dst = (char*) cblk->blk + i * cblk->unit;
      memcpy(dst, dev, cblk->unit);
      cblk->val[i] = 1;
      cblk->size++;
      cblk->front = cblk_front(cblk);
      cblk->back = cblk_back(cblk);
      return dst;
    }
  }

  size_t nextres = 2 * cblk->res;
  void* nextblk = realloc(cblk->blk, nextres * cblk->unit);
  uint64_t* nextval = realloc(cblk->val, 
    nextres * sizeof *nextval);
  if (nextblk == NULL || nextval == NULL)
    return NULL;

  memset(nextval + cblk->res, 0, nextres - cblk->res);
  cblk->blk = nextblk;
  cblk->val = nextval;
  cblk->res = nextres;
  void* dst = (char*) cblk->blk + cblk->size * cblk->unit;
  memcpy(dst, dev, cblk->unit);
  cblk->val[cblk->size] = 1;
  cblk->size++;
  cblk->front = cblk_front(cblk);
  cblk->back = cblk_back(cblk);
  return dst;
}

void
cblk_unmap(cblk_t* cblk, void* dev) {
  if (dev == NULL || cblk->size == 0)
    return;

  size_t const n = 
    ((char*) dev - (char*) cblk->blk) / cblk->unit;
  if (n >= cblk->res || cblk->val[n] == 0)
    return;

  cblk->val[n] = 0;
  cblk->size--;
  cblk->front = cblk_front(cblk);
  cblk->back = cblk_back(cblk);
}

void*
cblk_prev(cblk_t const* cblk, void* dev) {
  size_t const n = 
    ((char*) dev - (char*) cblk->blk) / cblk->unit;
  for (size_t i = 1; i <= cblk->res; i++) {
    size_t const j = (n + cblk->res - i) % cblk->res;
    if (cblk->val[j])
      return (char*) cblk->blk + j * cblk->unit;
  }

  return NULL;
}

void*
cblk_next(cblk_t const* cblk, void* dev) {
  size_t const n = 
    ((char*) dev - (char*) cblk->blk) / cblk->unit;
  for (size_t i = 1; i <= cblk->res; i++) {
    size_t const j = (i + n) % cblk->res;
    if (cblk->val[j])
      return (char*) cblk->blk + j * cblk->unit;
  }

  return NULL;
}

size_t const
cblk_dist(cblk_t const* cblk, 
    void const* dev) {
  /* dev must exist in cblk */
  void* dev_ = cblk->front;
  size_t n = 0;
  do {
    if (dev_ == dev)
      break;

    n++;
    dev_ = cblk_next(cblk, dev_);
  } while (dev_ != cblk->front);
  
  return n;
}

void
cblk_clear(cblk_t* cblk) {
  cblk->size = 0;
  cblk->front = cblk->back = NULL;
  memset(cblk->val, 0, cblk->res * sizeof *cblk->val);
}

void*
cblk_val_find(cblk_t const* cblk, void* val) {
  /* return here itr of comparison *val */
  void* dev = cblk->front;
  do {
    if (memcmp(dev, val, cblk->unit) == 0)
      return dev;

    dev = cblk_next(cblk, dev);
  } while (dev != cblk->front);

  return NULL;
}
