#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/cursorfont.h>
#include <stdio.h>

#include "font.h"

extern Display* dpy;
extern char const* font;

XftFont* xftfont;
/* Canonical width, height */
unsigned ch;
unsigned cw;
font_crs_t font_crs;

int font_init(void) {
  xftfont = XftFontOpenName(dpy, DefaultScreen(dpy), 
      font);
  if (!xftfont) {
    fprintf(stderr, "Failed to load font\n");
    return -1;
  }

  ch = 1.25 * xftfont->height;
  cw = 2.0 * xftfont->height;

  fprintf(stdout, "font height %d\n", xftfont->height);
  return 0;
}

void font_deinit(void) {
  XftFontClose(dpy, xftfont);
}

int font_cursor_init(void) {
  font_crs.ptr = XCreateFontCursor(dpy, XC_left_ptr);
  font_crs.move = XCreateFontCursor(dpy, XC_fleur);
  font_crs.resize = XCreateFontCursor(dpy, XC_sizing);
  font_crs.h = XCreateFontCursor(dpy, 
      XC_sb_h_double_arrow);
  font_crs.v = XCreateFontCursor(dpy, 
      XC_sb_v_double_arrow);
  return 0;
}

void font_cursor_deinit(void) {
  XFreeCursor(dpy, font_crs.v);
  XFreeCursor(dpy, font_crs.h);
  XFreeCursor(dpy, font_crs.resize);
  XFreeCursor(dpy, font_crs.move);
  XFreeCursor(dpy, font_crs.ptr);
}
