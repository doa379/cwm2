#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "clib.h"

ssize_t clib_min(ssize_t const v0, ssize_t const v1) {
  return v0 <= v1 ? v0 : v1;
}

ssize_t clib_max(ssize_t const v0, ssize_t const v1) {
  return v1 <= v0 ? v0 : v1;
}

int clib_rng(const unsigned N0, const unsigned N1) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
  return N0 + rand() % (N1 - N0 + 1);
}
