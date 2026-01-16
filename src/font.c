#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <locale.h>

#include "font.h"

extern Display* dpy;
extern char const* font_name;
font_t font;

static font_crs_t font_crs_init(void) {
  return (font_crs_t) {
    .ptr = XCreateFontCursor(dpy, XC_left_ptr),
    .move = XCreateFontCursor(dpy, XC_fleur),
    .resize = XCreateFontCursor(dpy, XC_sizing),
    .h = XCreateFontCursor(dpy, XC_sb_h_double_arrow),
    .v = XCreateFontCursor(dpy, XC_sb_v_double_arrow),
  };
}

static void font_crs_deinit(font_crs_t* const crs) {
  XFreeCursor(dpy, crs->v);
  XFreeCursor(dpy, crs->h);
  XFreeCursor(dpy, crs->resize);
  XFreeCursor(dpy, crs->move);
  XFreeCursor(dpy, crs->ptr);
}

int font_init(void) {
  font.xft = XftFontOpenName(dpy, DefaultScreen(dpy), 
      font_name);
  if (font.xft == NULL) {
    fprintf(stderr, "Failed to load font\n");
    return -1;
  }

  font.scent = font.xft->ascent - font.xft->descent;
  font.ch = 1.25 * font.xft->height;
  font.cw = 2.0 * font.xft->height;
  font.crs = font_crs_init();
  setlocale(LC_ALL, "");
  return 0;
}

void font_deinit(void) {
  font_crs_deinit(&font.crs);
  XftFontClose(dpy, font.xft);
}
