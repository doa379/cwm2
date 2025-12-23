#include <math.h>
#include <stdio.h>

#include "clr.h"

extern Display* dpy;
extern int const COLORS[];

clr_pair_t clr_pair[3];

static double
clr_linearize(int const val) {
  double const cs = val;
  return cs <= 0.03928 ? cs / 12.92 :
    pow((cs + 0.055) / 1.055, 2.4);
}

static double
clr_luminance(int const r, int const g, 
    int const b) {
/* Compute relative luminance (WCAG 2.0 formula) */
  return 0.2126 * clr_linearize(r) + 
      0.7152 * clr_linearize(g) + 
      0.0722 * clr_linearize(b);
}

static int
clr_fg(clr_t* const clr) {
  /* Calc fg for clr */
  return clr_luminance(clr->xr.red, clr->xr.green, 
    clr->xr.blue) > 0.5 ? 0x000000 : 0xffffff;
}

static clr_t
clr_pix_init(int const pix) {
  int const r = ((pix >> 16) & 0xff);
  int const g = ((pix >> 8) & 0xff);
  int const b = (pix & 0xff);
  return (clr_t) {
    .pix = pix,
    .xr = (XRenderColor) {
      .red = r,
      .green = g,
      .blue = b,
      .alpha = 65535
    },
  };
}

static int
clr_xft_init(clr_t* const clr) {
  if (XftColorAllocValue(dpy, 
      DefaultVisual(dpy, DefaultScreen(dpy)), 
      DefaultColormap(dpy, DefaultScreen(dpy)), 
      &clr->xr, &clr->xft) == False) {
    fprintf(stderr, "Failed to alloc clr\n");
    return -1;
  }

  return 0;
}

static void
clr_xft_deinit(clr_t* const clr) {
  XftColorFree(dpy, 
    DefaultVisual(dpy, DefaultScreen(dpy)), 
    DefaultColormap(dpy, DefaultScreen(dpy)), 
      &clr->xft);
}

int
clr_init(void) {
  for (int i = 0; i < 3; i++) {
    int const pix = COLORS[i];
    clr_pair[i].bg = clr_pix_init(pix);
    clr_xft_init(&clr_pair[i].bg);
    int const pix_fg = clr_fg(&clr_pair[i].bg);
    clr_pair[i].fg = clr_pix_init(pix_fg);
    clr_xft_init(&clr_pair[i].fg);
  }
  
  return 0;
}

void
clr_deinit(void) {
  for (int i = 0; i < 3; i++) {
    clr_xft_deinit(&clr_pair[i].bg);
    clr_xft_deinit(&clr_pair[i].fg);
  }
}
