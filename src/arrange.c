#include <math.h>

#include "arrange.h"

arrange_t
arrange_tile(size_t const n, size_t const N, 
  unsigned const dx, unsigned const dy, unsigned const W, 
  unsigned const H) {
  /* Constraint (W, H) */
  unsigned const nc = ceil(sqrt((float) N));
  unsigned const nr = (N + nc - 1) / nc;

  unsigned const total_dx = (nc + 1) * dx;
  unsigned const total_dy = (nr + 1) * dy;
  unsigned const w = (W - total_dx) / nc;
  unsigned const h = (H - total_dy) / nr;

  unsigned const col = n % nc;
  unsigned const row = n / nc;

  unsigned const x = dx + col * (w + dx);
  unsigned const y = dy + row * (h + dy);
  return (arrange_t) { .x = x, .y = y, .w = w, .h = h };
}

arrange_t
arrange_casc(size_t const n, unsigned const dx, 
unsigned const dy) {
  unsigned const x = n * dx;
  unsigned const y = n * dy;
  return (arrange_t) { .x = x, .y = y, .w = 0, .h = 0 };
}

arrange_t
arrange_mono(size_t const n, size_t const N,
  unsigned const dx, unsigned const dy, unsigned const W, 
  unsigned const H) {
  /* Constraint (W, H) */
  unsigned const w = W - N * dx;
  unsigned const h = H - N * dy;
  unsigned const x = n * dx;
  unsigned const y = n * dy;
  return (arrange_t) { .x = x, .y = y, .w = w, .h = h };
}
