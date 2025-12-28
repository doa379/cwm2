#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <stdio.h>

#include "mon.h"
#include "cblk.h"

extern Display* dpy;
cblk_t mons;

int
mon_mons_init(unsigned const n) {
  mons = cblk_init(sizeof(mon_t), n);
  if (mons.beg == NULL) {
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

static void*
mon_init(int const x, int const y, 
    int const w, int const h) {
  mon_t mon = {
    .x = x,
    .y = y,
    .w = w,
    .h = h
  };
  
  return cblk_map(&mons, &mon);
}

mon_t*
mon_currmon(int const x, int const y) {
  for (mon_t* mon = mons.beg; mon != mons.end; mon++)
    if (x > mon->x && x < mon->x + mon->w &&
      y > mon->y && y < mon->y + mon->h) {
      return mon;
    }

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
      for (int i = 0; i < n; i++)
        mon_init(inf[i].x_org, inf[i].y_org, 
          inf[i].width, inf[i].height);

      XFree(inf);
    }
  } else if (mons.size == 0)
    mon_init(0, 0, 
      DisplayWidth(dpy, DefaultScreen(dpy)),
      DisplayHeight(dpy, DefaultScreen(dpy)));
}
