#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>

#include "cblk.h"
#include "wg.h"

extern Display* dpy;

static size_t const NRES = 100;
static unsigned mode;
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
      /*
      (*wg)->x = X;
      (*wg)->y = 0;
      */
      x += (*wg)->w + 2 * (*wg)->bdrw + gap;
    }

    wg = cblk_next(&sel, wg);
  } while (wg != sel.front); 

  cblk_clear(&sel);
}

/* Arrange only on currently selected mon */

void
arrange_sel_tile(unsigned const w, unsigned const h) {
  /* Arrange within contraint (w, h) */
  size_t const n = sel.size;
  /* no of */
  unsigned nc = 0;
  for (; nc <= 0.5 * n; nc++)
    if (nc * nc >= n)
      break;

  if (n == 3)
    nc = 2;

  unsigned nr = n / nc;
  unsigned const currw = nc ? w / nc : w;
  /* (c, r) nos */
  unsigned cn = 0;
  unsigned rn = 0;
  wg_t** wg = sel.front; 
  do {
    size_t i = cblk_dist(&sel, *wg);
    if (i / nr + 1 > nc - n % nc)
      nr = n / nc + 1;

    unsigned const currh = nr ? w / nr : h;
    unsigned const currx = cn * currw;
    unsigned const curry = rn * currh;
    Window const win = (*wg)->win;
    if (wg_win_resize(*wg, currw, currh) == 0)
      XMoveWindow(dpy, (*wg)->win, currx, curry);

    rn++;
    if (rn >= nr) {
      rn = 0;
      cn++;
    }

    wg = cblk_next(&sel, wg);
  } while (wg != sel.front);

  cblk_clear(&sel);
}

void
arrange_sel_casc(unsigned const w, unsigned const h) {
  /* Arrange within contraint (w, h) */
  wg_t** wg = sel.front;
  do {

    wg = cblk_next(&sel, wg);
  } while (wg != sel.front);
  
  cblk_clear(&sel);
}
