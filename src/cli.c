#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include "font.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "arrange.h"

#include "res/cbox_16x16.xbm"
#include "res/cboxf_16x16.xbm"
#include "res/cboxff_16x16.xbm"
#include "res/cx_16x16.xbm"
#include "res/siz_16x16.xbm"
#include "res/cbox_24x24.xbm"
#include "res/cboxf_24x24.xbm"
#include "res/cboxff_24x24.xbm"
#include "res/cx_24x24.xbm"
#include "res/siz_24x24.xbm"

extern Display* dpy;

extern font_t font;
extern unsigned const bdrw;

static unsigned char const stipple_8x8[] = {
  /* Every 3rd bit */
  /* 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00 */
  0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55
};

static unsigned btnw;
static unsigned btnh;
static unsigned btnx;
static unsigned btny;
static unsigned char* BTN[5];

void
cli_wg_init(void) {
  BTN[MIN] = font.ch < 20 ? 
    cbox_16x16_bits : cbox_24x24_bits;
  BTN[MAX] = font.ch < 20 ? 
    cboxf_16x16_bits : cboxf_24x24_bits;
  BTN[RES] = font.ch < 20 ? 
    cboxff_16x16_bits : cboxff_24x24_bits;
  BTN[CLS] = font.ch < 20 ? 
    cx_16x16_bits : cx_24x24_bits;
  BTN[SIZ] = font.ch < 20 ? 
    siz_16x16_bits : siz_24x24_bits;
  btnh = btnw = font.ch < 20 ? 16 : 24;
  btnx = 0.5 * (font.cw - btnw);
  btny = 0.5 * (font.ch - btnh);
}

cli_t
cli_init(Window const win, wk_t* const wk, int const w, 
int const h) {
  /* Init parent */
  wg_t const par = wg_init(DefaultRootWindow(dpy), 
      w, h, bdrw);
  XReparentWindow(dpy, win, par.win, 0, font.ch);
  XSetWindowBorderWidth(dpy, win, 0);
  /* Init general header */
  wg_t hd0 = wg_init(par.win, 1, 1, 0);
  hd0.pixmap = XCreateBitmapFromData(dpy, hd0.win, 
    (char*) stipple_8x8, 8, 8);
  XSetStipple(dpy, hd0.gc, hd0.pixmap);
  XSetFillStyle(dpy, hd0.gc, FillStippled);
  /* Init special header */
  wg_t hd1 = wg_init(par.win, 1, 1, 0);
  XUnmapWindow(dpy, hd1.win);
  wg_win_bgset(hd1.win, wg_SEL);
  /* Init min */
  wg_t min = wg_init(hd0.win, btnw, btnh, 0);
  min.pixmap = XCreateBitmapFromData(dpy, min.win, 
    (char const*) BTN[MIN], btnw, btnh);
  /* Init max */
  wg_t max = wg_init(hd0.win, btnw, btnh, 0);
  max.pixmap = XCreateBitmapFromData(dpy, max.win, 
    (char const*) BTN[MAX], btnw, btnh);
  /* Init res */
  wg_t res = wg_init(hd0.win, btnw, btnh, 0);
  res.pixmap = XCreateBitmapFromData(dpy, res.win, 
    (char const*) BTN[RES], btnw, btnh);
  /* Init cls */
  wg_t cls = wg_init(hd0.win, btnw, btnh, 0);
  cls.pixmap = XCreateBitmapFromData(dpy, cls.win, 
    (char const*) BTN[CLS], btnw, btnh);
  /* Init siz */
  wg_t siz = wg_init(par.win, btnw, btnh, 0);
  siz.pixmap = XCreateBitmapFromData(dpy, siz.win, 
    (char const*) BTN[SIZ], btnw, btnh);
  /* Init ico */
  wg_t ico = wg_init(wk->wg.win, font.cw - 2 * bdrw, 
    font.ch - 2 * bdrw, bdrw);

  return (cli_t) {
    .wk = wk,
    .win = win,
    .par = par,
    .hd0 = hd0,
    .hd1 = hd1,
    .min = min,
    .max = max,
    .res = res,
    .cls = cls,
    .siz = siz,
    .ico = ico,
    .mode = RES,
    .sel = 0,
  };
}

void
cli_deinit(cli_t* const c) {
  wg_deinit(&c->siz);
  wg_deinit(&c->cls);
  wg_deinit(&c->res);
  wg_deinit(&c->max);
  wg_deinit(&c->min);
  wg_deinit(&c->hd1);
  wg_deinit(&c->hd0);
  wg_deinit(&c->par);
  wg_deinit(&c->ico);
  cblk_unmap(&c->wk->clis, c);
}

cli_t*
cli(Window const win, wk_t* const wk) {
  if (wk->clis.size == 0)
    return NULL;

  cli_t* c = wk->clis.front; 
  do {
    if (c->win == win ||
        c->par.win == win ||
        c->hd0.win == win ||
        c->min.win == win ||
        c->max.win == win ||
        c->res.win == win ||
        c->cls.win == win ||
        c->siz.win == win ||
        c->ico.win == win)
      return c;

    c = cblk_next(&wk->clis, c);
  } while (c != wk->clis.front); 
  
  return NULL;
}

wg_t*
cli_wg(cli_t* const c, Window const win) {
  return c->par.win == win ? &c->par :
    c->hd0.win == win ? &c->hd0 :
    c->min.win == win ? &c->min :
    c->max.win == win ? &c->max :
    c->res.win == win ? &c->res :
    c->cls.win == win ? &c->cls :
    c->siz.win == win ? &c->siz :
    &c->ico;
}

void
cli_wg_focus(cli_t* const c, unsigned const clr) {
  wg_win_bgset(c->par.win, clr);
  wg_win_bdrset(c->par.win, c->sel ? wg_SEL : clr);
  wg_win_bgset(c->hd0.win, clr);
  wg_gc_bgfgset(c->hd0.gc, clr);

  unsigned hd0 = btnw + 2 * c->par.bdrw;
  /* hd0 offset */
  XFillRectangle(dpy, c->hd0.win, c->hd0.gc,
    c->hd0.str.ext + 2 + hd0, 0, c->hd0.w, 
      font.ch - 2 * c->hd0.bdrw);
  wg_str_draw(&c->hd0, clr, hd0);

  wg_pixmap_fill(&c->min, clr);
  wg_pixmap_fill(&c->max, clr);
  wg_pixmap_fill(&c->res, clr);
  wg_pixmap_fill(&c->cls, clr);
  wg_pixmap_fill(&c->siz, clr);
  wg_win_bgset(c->ico.win, clr);
  wg_win_bdrset(c->ico.win, clr);
  wg_str_draw(&c->ico, clr, 0);
}

void
cli_conf(cli_t* const c, int const w, int const h) {
  /* Config elements given kernel dims */
  if (wg_win_resize(&c->par, w, h + font.ch) == 0) {
    if (wg_win_resize(&c->hd0, w, font.ch + c->par.bdrw) == 0) {
      wg_win_resize(&c->hd1, c->hd0.w, c->hd0.h);
      XMoveWindow(dpy, c->hd0.win, 0, 0);
      int const y0 = 0.5 * (c->hd0.h - btnh);
      /* Cls btn */
      XMoveWindow(dpy, c->cls.win, 0, y0);
      /* Max btn */
      wg_unmap(&c->max);
      /* Res btn */
      wg_unmap(&c->res);
      /* Res, Max selection */
      if (c->mode == MAX) {
        wg_map(&c->res);
        wg_unmap(&c->siz);
        XMoveWindow(dpy, c->res.win, 
          c->hd0.w - btnw, y0);
        /* Min btn */
        XMoveWindow(dpy, c->min.win, 
            c->hd0.w - 2 * btnw - 2 * bdrw, y0);
      } else {
        wg_map(&c->max);
        XMoveWindow(dpy, c->max.win, 
          c->hd0.w - 2 * btnw - 2 * bdrw, y0);
        wg_map(&c->siz);
        /* Min btn */
        XMoveWindow(dpy, c->min.win, 
            c->hd0.w - 3 * btnw - 4 * bdrw, y0);
      }

      /* Resize btn */
      XMoveWindow(dpy, c->siz.win, c->hd0.w - btnw, y0);
    }

    if (XResizeWindow(dpy, c->win, w, h)) {
      c->w = w;
      c->h = h;
    }
  }
}

void
cli_move(cli_t* const c, int const x, int const y) {
  /* if (x != c->x || y != c->y) */
  /* might want to mitigate server traffic */
    if (XMoveWindow(dpy, c->par.win, x, y)) {
      c->x0 = x;
      c->y0 = y;
      c->x1 = c->x0 + c->par.w + 2 * c->par.bdrw;
      c->y1 = c->y0 + c->par.h + 2 * c->par.bdrw;
    }
}

void
cli_resize(cli_t* const c, int const w, int const h) {
  /*
  if (wg_win_resize(&c->par, w, h) == 0) {
    cli_conf(c, w, h - font.ch);
    c->x1 = c->x + c->par.w + 2 * c->par.bdrw;
    c->y1 = c->y + c->par.h + 2 * c->par.bdrw;
  }
  */
  
  cli_conf(c, w, h - font.ch);
  c->x1 = c->x0 + c->par.w + 2 * c->par.bdrw;
  c->y1 = c->y0 + c->par.h + 2 * c->par.bdrw;
}

void
cli_anim(cli_t* const c, int const x, int const y, 
int const w, int const h, int const X, int const Y, 
int const W, int const H, int const d) {
  int const dX = (X - x) / d;
  int const dY = (Y - y) / d;
  int const dW = (W - w) / d;
  int const dH = (H - h) / d;
  wg_unmap(&c->hd0);
  XUnmapWindow(dpy, c->win);
  wg_win_bgset(c->par.win, wg_BG);
  for (int n = 0; n < d; n++) {
    int const nextX = x + dX * n;
    int const nextY = y + dY * n;
    int const nextW = w + dW * n;
    int const nextH = h + dH * n;
    XResizeWindow(dpy, c->par.win, nextW, nextH);
    XMoveWindow(dpy, c->par.win, nextX, nextY);
    XFlush(dpy);
    usleep(1000);
  }

  /*
  XResizeWindow(dpy, c->par.win, W, H);
  XMoveWindow(dpy, c->par.win, X, Y);
  */
  
  wg_win_bgset(c->par.win, wg_ACT);
  XMapWindow(dpy, c->win);
  wg_map(&c->hd0);
}
