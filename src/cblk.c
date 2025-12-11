#include <stdlib.h>
#include <string.h>

#include "cblk.h"

cblk_t cblk_init(size_t const unit, size_t const res) {
  void* blk = malloc(unit * (res + 1));
  return (cblk_t) {
    .beg = blk,
    .end = blk,
    .unit = unit,
    .size = 0,
    .res = res
  };
}

void cblk_deinit(cblk_t* cblk) {
  free(cblk->beg);
}

void* cblk_beg(cblk_t const* cblk) {
  return cblk->beg;
}

void* cblk_end(cblk_t const* cblk) {
  return (char*) cblk->beg + cblk->unit * cblk->size;
}

void* cblk_back(cblk_t const* cblk) {
  return cblk->size > 0 ?
    (char*) cblk->end - cblk->unit : NULL;
}

void* cblk_itr(cblk_t const* cblk, ssize_t const n) {
  if (n >= 0) {
    void* p = (char*) cblk->beg + n * cblk->unit;
    return p < cblk->end ? p : NULL;
  }

  return NULL;
}

void* cblk_map(cblk_t* cblk, void const* dev) {
  /* Returns address of dev */
  if (cblk->size < cblk->res) {
    if (memcpy((char*) cblk->beg + cblk->unit * cblk->size, 
        dev, cblk->unit)) {
      cblk->size++;
      cblk->end = cblk_end(cblk);
      return (char*) cblk->end - cblk->unit;
    }
  } else {
    size_t res = cblk->size + cblk->res;
    void* ptr = realloc(cblk->beg, cblk->unit * (res + 1));
    if (ptr) {
      cblk->beg = ptr;
      cblk->res = res;
      if (memcpy((char*) cblk->beg + 
            cblk->unit * cblk->size, dev, cblk->unit)) {
        cblk->size++;
        cblk->end = cblk_end(cblk);
        return (char*) cblk->end - cblk->unit;
      }
    }
  }

  return NULL;
}

void cblk_unmap(cblk_t* cblk, void* dev) {
  /*
  char const* p = (char*) dev + cblk->unit;
  memmove(dev, p, (char*) cblk->end - p);
  cblk->size--;
  cblk->end = cblk_end(cblk);
  */

  if (cblk->size == 0 || dev == NULL)
    return;

  char* d = (char*) dev;
  char const* e = (char*) cblk->end;
  if (d < (char*) cblk->beg || d >= e)
      /* invalid pointer */
      return;

  /* move everything after `dev` one slot left */
  memmove(d, d + cblk->unit, e - (d + cblk->unit));
  cblk->size--;
  cblk->end = (char*) cblk->end - cblk->unit;
}

void* cblk_find(cblk_t* cblk, void const* dev) {
  char* p;
  for (p = cblk->beg; p < (char*) cblk->end; 
      p += cblk->unit)
    if (p == dev)
        return p;

  return NULL;
}

void* cblk_prev(cblk_t* cblk, void* dev) {
  return cblk->size == 0 ? NULL :
    cblk->size == 1 ? dev :
      dev == cblk->beg ? (char*) cblk->end - cblk->unit : 
        (char*) dev - cblk->unit;
}

void* cblk_next(cblk_t* cblk, void* dev) {
  return cblk->size == 0 ? NULL :
    cblk->size == 1 ? dev :
      dev == (char*) cblk->end - cblk->unit ? cblk->beg :
        (char*) dev + cblk->unit;
}

size_t const cblk_dist(cblk_t const* cblk, 
    void const* dev) {
  return ((char*) dev - (char*) cblk->beg) / cblk->unit;
}

void cblk_clear(cblk_t* cblk) {
  cblk->size = 0;
  cblk->end = cblk->beg;
}
