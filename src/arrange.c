#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "cblk.h"
#include "wg.h"

extern Display* dpy;

static size_t const NRES = 100;
static unsigned mode;
static cblk_t sel;

int arrange_init(void) {
  sel = cblk_init(sizeof(wg_t*), NRES);
  if (sel.beg == NULL) {
    fprintf(stderr, "Failed to alloc arrangements\n");
    return -1;
  }

  return 0;
}

void arrange_deinit(void) {
  cblk_deinit(&sel); 
}

void arrange_sel_map(wg_t const* wg) {
  cblk_map(&sel, &wg);
}

void arrange_sel_clear(void) {
  cblk_clear(&sel);
}

void arrange_sel_adj(int const gap) {
  int x = 0;
  for (wg_t** wg = sel.beg; wg != sel.end; wg++) {
    int const X = x;
    if (XMoveWindow(dpy, (*wg)->win, X, 0)) {
      (*wg)->x = X;
      (*wg)->y = 0;
      x += (*wg)->w + 2 * (*wg)->bdrw + gap;
    }
  }

  cblk_clear(&sel);
}
/* Arrange only on currently selected mon */
void arrange_sel_tile(void) {
  for (wg_t** wg = sel.beg; wg != sel.end; wg++)
    ;

  cblk_clear(&sel);
}

void arrange_sel_casc(void) {
  for (wg_t** wg = sel.beg; wg != sel.end; wg++)
    ;
  
  cblk_clear(&sel);
}
