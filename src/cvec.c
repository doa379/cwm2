#include <stdlib.h>
#include <string.h>

#include "cvec.h"

cvec_t cvec_init(size_t const unit, size_t const res) {
  void* blk = malloc(unit * (res + 1));
  return (cvec_t) {
    .beg = blk,
    .end = blk,
    .unit = unit,
    .size = 0,
    .res = res,
  };
}

void cvec_deinit(cvec_t* cvec) {
  free(cvec->beg);
}

void* cvec_beg(cvec_t const* cvec) {
  return cvec->beg;
}

void* cvec_end(cvec_t const* cvec) {
  return (char*) cvec->beg + cvec->unit * cvec->size;
}

void* cvec_back(cvec_t const* cvec) {
  return cvec->size > 0 ?
    (char*) cvec->end - cvec->unit : NULL;
}

void* cvec_itr(cvec_t const* cvec, size_t const n) {
  void* p = (char*) cvec->beg + n * cvec->unit;
  return p < cvec->end ? p : NULL;
}

void* cvec_map(cvec_t* cvec, void const* dev) {
  /* Returns address of dev */
  if (cvec->size < cvec->res) {
    if (memcpy((char*) cvec->beg + cvec->unit * cvec->size, 
        dev, cvec->unit)) {
      cvec->size++;
      cvec->end = cvec_end(cvec);
      return (char*) cvec->end - cvec->unit;
    }
  } else {
    size_t res = cvec->size + cvec->res;
    void* ptr = realloc(cvec->beg, cvec->unit * (res + 1));
    if (ptr) {
      cvec->beg = ptr;
      cvec->res = res;
      if (memcpy((char*) cvec->beg + 
            cvec->unit * cvec->size, dev, cvec->unit)) {
        cvec->size++;
        cvec->end = cvec_end(cvec);
        return (char*) cvec->end - cvec->unit;
      }
    }
  }

  return NULL;
}

void cvec_unmap(cvec_t* cvec, void* dev) {
  if (cvec->size == 0 || dev == NULL)
    return;

  char* d = (char*) dev;
  char const* e = (char*) cvec->end;
  if (d < (char*) cvec->beg || d >= e)
      return;

  memmove(d, d + cvec->unit, e - (d + cvec->unit));
  cvec->size--;
  cvec->end = (char*) cvec->end - cvec->unit;
}

void const* cvec_find(cvec_t* cvec, void const* dev) {
  return dev >= cvec->beg && dev < cvec->end ? dev : NULL;
}

void* cvec_prev(cvec_t* cvec, void* dev) {
  return cvec->size == 0 ? NULL :
    cvec->size == 1 ? dev :
      dev == cvec->beg ? (char*) cvec->end - cvec->unit : 
        (char*) dev - cvec->unit;
}

void* cvec_next(cvec_t* cvec, void* dev) {
  return cvec->size == 0 ? NULL :
    cvec->size == 1 ? dev :
      dev == (char*) cvec->end - cvec->unit ? cvec->beg :
        (char*) dev + cvec->unit;
}

size_t const cvec_dist(cvec_t const* cvec, 
    void const* dev) {
  return ((char*) dev - (char*) cvec->beg) / cvec->unit;
}

void cvec_clear(cvec_t* cvec) {
  cvec->size = 0;
  cvec->end = cvec->beg;
}
