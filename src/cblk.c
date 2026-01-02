#include <stdlib.h>
#include <string.h>

#include "cblk.h"

cblk_t cblk_init(size_t const unit, size_t const res) {
  void* blk = malloc(unit * (res + 1));
  unsigned char* val = calloc(res, sizeof *val);
  return (cblk_t) {
    .beg = blk,
    .end = blk,
    .unit = unit,
    .size = 0,
    .res = res,
    .val = val,
  };
}

void cblk_deinit(cblk_t* cblk) {
  free(cblk->val);
  free(cblk->beg);
}

void* cblk_beg(cblk_t const* cblk) {
  /*
  return cblk->beg;
  */
  for (size_t i = 0; i < cblk->res; i++)
    if (cblk->val[i])
      return (char*) cblk->beg + i * cblk->unit;

  return NULL;
}

void* cblk_end(cblk_t const* cblk) {
  /* return (char*) cblk->beg + cblk->unit * cblk->size; */
  return (char*) cblk->beg + cblk->unit * cblk->res;
}

void* cblk_back(cblk_t const* cblk) {
  /*
  return cblk->size > 0 ?
    (char*) cblk->end - cblk->unit : NULL;
  */
  for (size_t i = cblk->res - 1; i >= 0; i--)
    if (cblk->val[i])
      return (char*) cblk->beg + i * cblk->unit;
      
  return NULL;
}

void* cblk_itr(cblk_t const* cblk, ssize_t const n) {
  /*
  if (n >= 0) {
    void* p = (char*) cblk->beg + n * cblk->unit;
    return p < cblk->end ? p : NULL;
  }

  return NULL;
  */

  size_t c = 0;
  for (size_t i = 0; i < cblk->res; i++) {
    c += cblk->val[i];
    if (c == n)
      return (char*) cblk->beg + (c - 1) * cblk->unit;
  }

  return NULL;
}

void* cblk_map(cblk_t* cblk, void const* dev) {
  /* Returns address of dev */
  /*
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
  */

  for (size_t i = 0; i < cblk->res; i++) {
    if (cblk->val[i] == 0) {
      void* dst = (char*) cblk->beg + i * cblk->unit;
      memcpy(dst, dev, cblk->unit);
      cblk->val[i] = 1;
      cblk->size++;
      if (dst >= cblk->end)
        cblk->end = (char*) dst + cblk->unit;

      return dst;
    }
  }

  size_t nextres = 2 * cblk->res + 1;
  void* nbeg = realloc(cblk->beg, nextres * cblk->unit);
  unsigned char* nval = realloc(cblk->val, nextres);
  if (nbeg == NULL || nval == NULL)
    return NULL;

  memset(nval + cblk->res, 0, nextres - cblk->res);
  cblk->beg = nbeg;
  cblk->val = nval;
  cblk->res = nextres;
  void* dst = (char*) cblk->beg + cblk->size * cblk->unit;
  memcpy(dst, dev, cblk->unit);
  cblk->val[cblk->size] = 1;
  cblk->size++;
  cblk->end = (char*)dst + cblk->unit;
  return dst;
}

void cblk_unmap(cblk_t* cblk, void* dev) {
  /*
  if (cblk->size == 0 || dev == NULL)
    return;

  char* d = (char*) dev;
  char const* e = (char*) cblk->end;
  if (d < (char*) cblk->beg || d >= e)
      return;

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
  */

  if (dev == NULL || cblk->size == 0)
    return;

  size_t i = ((char*) dev - (char*) cblk->beg) / cblk->unit;
  if (i >= cblk->res || cblk->val[i] == 0)
    return;

  cblk->val[i] = 0;
  cblk->size--;
}

void* cblk_prev(cblk_t* cblk, void* dev) {
  /*
  return cblk->size == 0 ? NULL :
    cblk->size == 1 ? dev :
      dev == cblk->beg ? (char*) cblk->end - cblk->unit : 
        (char*) dev - cblk->unit;
  */

  if (cblk->size == 0)
    return NULL;

  size_t n = ((char*) dev - (char*) cblk->beg) / cblk->unit;
  for (size_t i = 1; i <= cblk->res; i++) {
    size_t j = (i + cblk->res - n) % cblk->res;
    if (cblk->val[j])
      return (char*) cblk->beg + j * cblk->unit;
  }

  return NULL;
}

void* cblk_next(cblk_t* cblk, void* dev) {
  /*
  return cblk->size == 0 ? NULL :
    cblk->size == 1 ? dev :
      dev == (char*) cblk->end - cblk->unit ? cblk->beg :
        (char*) dev + cblk->unit;
  */
  
  if (cblk->size == 0)
    return NULL;

  size_t n = ((char*) dev - (char*) cblk->beg) / cblk->unit;
  for (size_t i = 1; i <= cblk->res; i++) {
    size_t j = (i + n) % cblk->res;
    if (cblk->val[j])
      return (char*) cblk->beg + j * cblk->unit;
  }

  return NULL;
}

ssize_t const cblk_dist(cblk_t const* cblk, 
    void const* dev) {
  /*
  return ((char*) dev - (char*) cblk->beg) / cblk->unit;
  */
  
  size_t c = 0;
  for (size_t i = 0; i < cblk->res; i++) {
    c += cblk->val[i];
    if ((char*) cblk->beg + c * cblk->unit == dev)
      return c - 1;
  }

  return -1;
}

void cblk_clear(cblk_t* cblk) {
  cblk->size = 0;
  cblk->end = cblk->beg;
  memset(cblk->val, 0, cblk->res * sizeof *cblk->val);
}
