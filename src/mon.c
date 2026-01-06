#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <stdio.h>

#include "mon.h"
#include "cblk.h"
#include "tray.h"
#include "panel.h"

extern Display* dpy;
extern tray_t tray;
extern wg_t panel;

static size_t const NRES = 1;

cblk_t mons;

int
mon_mons_init(void) {
  mons = cblk_init(sizeof(mon_t), NRES);
  if (mons.blk == NULL) {
    fprintf(stderr, "Failed to init mon\n");
    return -1;
  }

  return 0;
}

void
mon_mons_deinit(void) {
  cblk_deinit(&mons);
}

void
mon_mons_clear(void) {
  cblk_clear(&mons);
}

mon_t*
mon_currmon(int const x, int const y) {
  mon_t* mon = mons.front;
  do {
    if (x > mon->x && x < mon->x + mon->w &&
      y > mon->y && y < mon->y + mon->h) {
      return mon;
    }

    mon = cblk_next(&mons, mon);
  } while (mon != mons.front); 

  return NULL;
}

void
mon_conf(void) {
  if (XineramaIsActive(dpy)) {
    int n = 0;
    XineramaScreenInfo* inf =
      XineramaQueryScreens(dpy, &n);
    if (inf) {
      cblk_clear(&mons);
      for (int i = 0; i < n; i++) {
        mon_t const mon = {
          .x = inf[i].x_org,
          .y = inf[i].y_org,
          .w = inf[i].width,
          .h = inf[i].height
        };
        
        cblk_map(&mons, &mon);
      }

      XFree(inf);
    }
  } else {
    /* Mandatory at least 1x mon */
      mon_t const mon = {
        .x = 0,
        .y = 0,
        .w = DisplayWidth(dpy, DefaultScreen(dpy)),
        .h = DisplayHeight(dpy, DefaultScreen(dpy))
      };
      
      cblk_map(&mons, &mon);
  }
}
