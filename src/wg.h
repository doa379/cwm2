#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef struct {
  char data[26];
  unsigned len;
  short ext;
} wg_str_t;

typedef struct {
/* Widget type */
  Window win;
  int w;
  int h;
  int bdrw;
  unsigned long mask;
  Pixmap pixmap;
  GC gc;
  XftDraw* xft;
  wg_str_t str;
} wg_t;

enum colors { wg_BG, wg_ACT, wg_SEL };

wg_t wg_init(Window const parwin, int const, int const, 
  int const);
void wg_deinit(wg_t* const);

void wg_str_set(wg_t* const, char const*);
void wg_str_draw(wg_t* const, unsigned const, 
    unsigned const);

void wg_win_bgset(Window const, unsigned const);
void wg_win_bdrset(Window const, unsigned const);
void wg_gc_bgset(GC const, unsigned const);
void wg_pixmap_fill(wg_t const*, unsigned const);
int wg_win_move(wg_t* const, int const, int const);
int wg_win_resize(wg_t* const, int const, int const);
