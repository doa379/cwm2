#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>

#include "cblk.h"
#include "wg.h"

extern Display* dpy;

static size_t const NRES = 100;
static cblk_t sel;

int
arrange_init(void) {
  sel = cblk_init(sizeof(wg_t*), NRES);
  if (sel.blk == NULL) {
    fprintf(stderr, "Failed to alloc arrangements\n");
    return -1;
  }

  return 0;
}

void
arrange_deinit(void) {
  cblk_deinit(&sel); 
}

void
arrange_sel_map(wg_t const* wg) {
  cblk_map(&sel, &wg);
}

void
arrange_sel_clear(void) {
  cblk_clear(&sel);
}

void
arrange_sel_adj(int const gap) {
  int x = 0;
  wg_t** wg = sel.front; 
  do {
    int const X = x;
    if (XMoveWindow(dpy, (*wg)->win, X, 0)) {
      x += (*wg)->w + 2 * (*wg)->bdrw + gap;
    }

    wg = cblk_next(&sel, wg);
  } while (wg != sel.front); 
  cblk_clear(&sel);
}

void
arrange_sel_tile(unsigned const w, unsigned const h) {
  /* Arrange within constraint (w, h) */
  size_t const n = sel.size;
  unsigned const nc = ceil(sqrt(n));
  unsigned const nr = (n + nc - 1) / nc;
  unsigned const cellw = w / nc;
  unsigned const cellh = h / nr;
  unsigned i = 0;
  wg_t** wg = sel.front;
  do {
    unsigned const col = i % nc;
    unsigned const row = i / nc;
    unsigned const x = col * cellw;
    unsigned const y = row * cellh;
    wg_win_resize(*wg, cellw, cellh);
    XMoveWindow(dpy, (*wg)->win, x, y);
    i++;
    wg = cblk_next(&sel, wg);
  } while (wg != sel.front);
  cblk_clear(&sel);
}

void
arrange_sel_casc(unsigned const w, unsigned const h) {
  /* Arrange within contraint (w, h) */
  size_t const n = sel.size;
  unsigned const m = 16;
  unsigned const cellw = w - m * n;
  unsigned const cellh = h - m * n;
  wg_t** wg = sel.front;
  int x = 0;
  int y = 0;
  do {
    wg_win_resize(*wg, cellw, cellh);
    XMoveWindow(dpy, (*wg)->win, x, y);
    x += m;
    y += m;
    wg = cblk_next(&sel, wg);
  } while (wg != sel.front);
  cblk_clear(&sel);
}
