#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>

extern Display* dpy;
extern char const* font;

XftFont* xftfont;
/* Canonical width, height */
unsigned ch;
unsigned cw;

int font_init(void) {
  xftfont = XftFontOpenName(dpy, DefaultScreen(dpy), font);
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
