#include "cblk.h"
#include "wg.h"

static size_t const NRES = 100;
cblk_t sel;

int
sel_init(void) {
  sel = cblk_init(sizeof(wg_t*), NRES);
  if (sel.blk == NULL) {
    fprintf(stderr, "Failed to alloc selections\n");
    return -1;
  }

  return 0;
}

void
sel_deinit(void) {
  cblk_deinit(&sel); 
}

void
sel_map(wg_t const* wg) {
  cblk_map(&sel, &wg);
}

void
sel_unmap(wg_t const* wg) {
  cblk_unmap(&sel, &wg);
}

void
sel_clear(void) {
  cblk_clear(&sel);
}

wg_t*
sel_find(wg_t const* wg) {
  if (sel.size == 0) {
    return NULL;
  }

  wg_t** dev = sel.front;
  do {
    if (*dev == wg) {
      return *dev;
    }

    dev = cblk_next(&sel, dev);
  } while (dev != sel.front);

  return NULL;
}
