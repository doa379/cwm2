#include <math.h>
#include <stdio.h>

#include "clr.h"

extern Display* dpy;
extern int const COLORS[];

clr_t CLR[5];

static double clr_linearize(int const val) {
  double const cs = val;
  return cs <= 0.03928 ? cs / 12.92 :
    pow((cs + 0.055) / 1.055, 2.4);
}

static double clr_luminance(int const r, int const g, 
    int const b) {
/* Compute relative luminance (WCAG 2.0 formula) */
  return 0.2126 * clr_linearize(r) + 
      0.7152 * clr_linearize(g) + 
      0.0722 * clr_linearize(b);
}

int clr_init(void) {
  for (int i = 0; i < 3; i++) {
    int const pix = COLORS[i];
    clr_t* clr = &CLR[i];
    clr->pix = pix;
    clr->xr = (XRenderColor) {
      .red = ((pix >> 16) & 0xff),
      .green = ((pix >> 8) & 0xff),
      .blue = (pix & 0xff),
      .alpha = 65535
    };
    
    if (!XftColorAllocValue(dpy, 
        DefaultVisual(dpy, DefaultScreen(dpy)), 
        DefaultColormap(dpy, DefaultScreen(dpy)), 
        &clr->xr, &clr->xft)) {
      fprintf(stderr, "Failed to alloc clr\n");
      return -1;
    }

    /* Alternative alloc as string fmt hex value */
    /*
    if (!XftColorAllocName(dpy, 
          DefaultVisual(dpy, DefaultScreen(dpy)), 
          DefaultColormap(dpy, DefaultScreen(dpy)), 
          "#000000", clr->xft))
      ;
    */
  }

  {
    /* Calc fg for BG */
    int const bg = COLORS[0];
    int const r = ((bg >> 16) & 0xff);
    int const g = ((bg >> 8) & 0xff);
    int const b = (bg & 0xff);
    int const fg = clr_luminance(r, g, b) > 0.5 ? 
      0x000000 : 0xffffff;
    clr_t* clr = &CLR[3];
    clr->pix = fg;
    clr->xr = (XRenderColor) {
      .red = r,
      .green = g,
      .blue = b,
      .alpha = 65535
    };
    
    if (!XftColorAllocValue(dpy, 
        DefaultVisual(dpy, DefaultScreen(dpy)), 
        DefaultColormap(dpy, DefaultScreen(dpy)), 
        &clr->xr, &clr->xft)) {
      fprintf(stderr, "Failed to alloc clr\n");
      return -1;
    }
  }
  
  {
    /* Calc fg for ACT */
    int const act = COLORS[1];
    int const r = ((act >> 16) & 0xff);
    int const g = ((act >> 8) & 0xff);
    int const b = (act & 0xff);
    int const fg = clr_luminance(r, g, b) > 0.5 ? 
      0x000000 : 0xffffff;
    clr_t* clr = &CLR[4];
    clr->pix = fg;
    clr->xr = (XRenderColor) {
      .red = r,
      .green = g,
      .blue = b,
      .alpha = 65535
    };
    
    if (!XftColorAllocValue(dpy, 
        DefaultVisual(dpy, DefaultScreen(dpy)), 
        DefaultColormap(dpy, DefaultScreen(dpy)), 
        &clr->xr, &clr->xft)) {
      fprintf(stderr, "Failed to alloc clr\n");
      return -1;
    }
  }

  return 0;
}

void clr_deinit(void) {
  for (int i = 0; i < 5; i++)
    XftColorFree(dpy, 
      DefaultVisual(dpy, DefaultScreen(dpy)), 
      DefaultColormap(dpy, DefaultScreen(dpy)), 
        &CLR[i].xft);
}

