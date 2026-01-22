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

static long const KMASK = NoEventMask;
static long const CLIMASK = 
  FocusChangeMask | 
  PropertyChangeMask |
  EnterWindowMask |
  SubstructureNotifyMask |
  SubstructureRedirectMask;
static long const HDRMASK =
  EnterWindowMask |
  ButtonPressMask |
  ButtonReleaseMask |
  ExposureMask;
static long const BTNMASK = 
  EnterWindowMask |
  LeaveWindowMask |
  ButtonPressMask |
  ButtonReleaseMask |
  ExposureMask;
static unsigned bdrw_twice;
static unsigned btnw;
static unsigned btnh;
static unsigned btnx;
static unsigned btny;
static unsigned char* BTN[5];

void
cli_wg_init(void) {
  bdrw_twice = 2 * bdrw;
  BTN[cli_MIN] = font.ch < 20 ? 
    cbox_16x16_bits : cbox_24x24_bits;
  BTN[cli_MAX] = font.ch < 20 ? 
    cboxf_16x16_bits : cboxf_24x24_bits;
  BTN[cli_RES] = font.ch < 20 ? 
    cboxff_16x16_bits : cboxff_24x24_bits;
  BTN[cli_CLS] = font.ch < 20 ? 
    cx_16x16_bits : cx_24x24_bits;
  BTN[cli_SIZ] = font.ch < 20 ? 
    siz_16x16_bits : siz_24x24_bits;
  btnh = btnw = font.ch < 20 ? 16 : 24;
  btnx = 0.5 * (font.cw - btnw);
  btny = 0.5 * (font.ch - btnh);
}

cli_t
cli_init(Window const win, wk_t* const wk) {
  /* Init parent */
  wg_t const par = wg_init(DefaultRootWindow(dpy), 
      1, 1, bdrw);
  wg_win_bgclr(par.win, wg_BG);
  XReparentWindow(dpy, win, par.win, 0, 0);
  XSelectInput(dpy, par.win, CLIMASK);
  XSetWindowBorderWidth(dpy, win, 0);
  XMapWindow(dpy, win);
  XSelectInput(dpy, win, KMASK);
  /* Init general header */
  wg_t hd0 = wg_init(par.win, 1, 1, 0);
  hd0.pixmap = XCreateBitmapFromData(dpy, hd0.win, 
    (char*) stipple_8x8, 8, 8);
  XSetStipple(dpy, hd0.gc, hd0.pixmap);
  XSetFillStyle(dpy, hd0.gc, FillStippled);
  XSelectInput(dpy, hd0.win, HDRMASK);
  /* Init special header */
  wg_t hd1 = wg_init(par.win, 1, 1, 0);
  wg_win_bgclr(hd1.win, wg_SEL);
  XUnmapWindow(dpy, hd1.win);
  /* Init min */
  wg_t min = wg_init(hd0.win, btnw, btnh, 0);
  min.pixmap = XCreateBitmapFromData(dpy, min.win, 
    (char const*) BTN[cli_MIN], btnw, btnh);
  XSelectInput(dpy, min.win, BTNMASK);
  /* Init max */
  wg_t max = wg_init(hd0.win, btnw, btnh, 0);
  max.pixmap = XCreateBitmapFromData(dpy, max.win, 
    (char const*) BTN[cli_MAX], btnw, btnh);
  XSelectInput(dpy, max.win, BTNMASK);
  /* Init res */
  wg_t res = wg_init(hd0.win, btnw, btnh, 0);
  res.pixmap = XCreateBitmapFromData(dpy, res.win, 
    (char const*) BTN[cli_RES], btnw, btnh);
  XSelectInput(dpy, res.win, BTNMASK);
  /* Init cls */
  wg_t cls = wg_init(hd0.win, btnw, btnh, 0);
  cls.pixmap = XCreateBitmapFromData(dpy, cls.win, 
    (char const*) BTN[cli_CLS], btnw, btnh);
  XSelectInput(dpy, cls.win, BTNMASK);
  /* Init siz */
  wg_t siz = wg_init(hd0.win, btnw, btnh, 0);
  siz.pixmap = XCreateBitmapFromData(dpy, siz.win, 
    (char const*) BTN[cli_SIZ], btnw, btnh);
  XSelectInput(dpy, siz.win, BTNMASK);
  /* Init ico */
  wg_t ico = wg_init(wk->wg.win, 
    1.25 * font.cw - bdrw_twice, font.ch - bdrw_twice, 
      bdrw);
  XSelectInput(dpy, ico.win, BTNMASK);
  /* */
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
    .mode = cli_RES,
    .sel = 0,
  };
}

void
cli_deinit(cli_t* const c) {
  XSelectInput(dpy, c->ico.win, NoEventMask);
  wg_deinit(&c->ico);
  XSelectInput(dpy, c->siz.win, NoEventMask);
  wg_deinit(&c->siz);
  XSelectInput(dpy, c->cls.win, NoEventMask);
  wg_deinit(&c->cls);
  XSelectInput(dpy, c->res.win, NoEventMask);
  wg_deinit(&c->res);
  XSelectInput(dpy, c->max.win, NoEventMask);
  wg_deinit(&c->max);
  XSelectInput(dpy, c->min.win, NoEventMask);
  wg_deinit(&c->min);
  XSelectInput(dpy, c->hd1.win, NoEventMask);
  wg_deinit(&c->hd1);
  XSelectInput(dpy, c->hd0.win, NoEventMask);
  wg_deinit(&c->hd0);
  XMapWindow(dpy, c->par.win);
  XSelectInput(dpy, c->win, NoEventMask);
  XReparentWindow(dpy, c->win, DefaultRootWindow(dpy), 
    c->x0, c->y0);
  XSelectInput(dpy, c->par.win, NoEventMask);
  wg_deinit(&c->par);
}

cli_t*
cli(Window const win, wk_t* const wk) {
  if (wk->clis.size == 0) {
    return NULL;
  }

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
        c->ico.win == win) {
      return c;
    }

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
  wg_win_bdrclr(c->par.win, c->sel ? wg_SEL : clr);
  unsigned hd0 = btnw + bdrw_twice;
  /* hd0 offset */
  wg_str_draw(&c->hd0, clr, hd0);
  XFillRectangle(dpy, c->hd0.win, c->hd0.gc,
    c->hd0.str.ext + 2 + hd0, 0, c->hd0.w,
      c->hd0.h - 2 * c->hd0.bdrw);

  wg_pixmap_fill(&c->min, clr);
  wg_pixmap_fill(&c->max, clr);
  wg_pixmap_fill(&c->res, clr);
  wg_pixmap_fill(&c->cls, clr);
  wg_pixmap_fill(&c->siz, clr);
  wg_win_bgclr(c->ico.win, clr);
  wg_win_bdrclr(c->ico.win, clr);
  wg_str_draw(&c->ico, clr, 0);
}

void
cli_conf(cli_t* const c, int const w, int const h) {
  /* Config elements given kernel dims */
  wg_win_resize(&c->par, w, h + font.ch);
  wg_win_resize(&c->hd0, c->par.w, font.ch);
  wg_win_resize(&c->hd1, c->hd0.w, c->hd0.h);
  XMoveWindow(dpy, c->hd0.win, 0, 0);
  int const y0 = 0.5 * (c->hd0.h - btnh);
  /* Cls btn */
  XMoveWindow(dpy, c->cls.win, 0, y0);
  /* Max btn */
  XUnmapWindow(dpy, c->max.win);
  /* Res btn */
  XUnmapWindow(dpy, c->res.win);
  /* Res, Max selection */
  if (c->mode == cli_MAX) {
    XMapWindow(dpy, c->res.win);
    XUnmapWindow(dpy, c->siz.win);
    XMoveWindow(dpy, c->res.win, c->hd0.w - btnw, y0);
    /* Min btn */
    XMoveWindow(dpy, c->min.win, 
        c->hd0.w - 2 * btnw - bdrw_twice, y0);
  } else {
    XMapWindow(dpy, c->max.win);
    XMoveWindow(dpy, c->max.win, 
      c->hd0.w - 2 * btnw - bdrw_twice, y0);
    XMapWindow(dpy, c->siz.win);
    /* Min btn */
    XMoveWindow(dpy, c->min.win, 
        c->hd0.w - 3 * btnw - 2 * bdrw_twice, y0);
  }

  /* Resize btn */
  XMoveWindow(dpy, c->siz.win, c->hd0.w - btnw, y0);
  /* Kernel */
  if (XResizeWindow(dpy, c->win, w, h)) {
    c->w = w;
    c->h = h;
  }

  XMoveWindow(dpy, c->win, 0, c->hd0.h);
}

void
cli_move(cli_t* const c, int const x, int const y, 
int const W, int const H) {
  /* Constraint (W, H) */
  if (XMoveWindow(dpy, c->par.win, x, y)) {
    c->x0 = x;
    c->y0 = y;
  }

  int const cw = c->par.w + bdrw_twice;
  if (c->x0 + cw > W) {
    int const x = W - cw;
    if (XMoveWindow(dpy, c->par.win, x, c->y0)) {
      c->x0 = x;
    }
  }

  int const ch = c->par.h + bdrw_twice;
  if (c->y0 + ch > H) {
    int const y = H - ch;
    if (XMoveWindow(dpy, c->par.win, c->x0, y)) {
      c->y0 = y;
    }
  }
}

void
cli_resize(cli_t* const c, int const w, int const h,
int const W, int const H) {
  /* Constraint (W, H) */
  int const cw = w + bdrw_twice > W ? W : w;
  int const ch = h + font.ch + bdrw_twice > H ? H : h;
  cli_conf(c, cw, ch);
}

void
cli_anim(cli_t* const c, int const x, int const y, 
int const w, int const h, int const X, int const Y, 
int const W, int const H, int const d) {
  int const dX = (X - x) / d;
  int const dY = (Y - y) / d;
  int const dW = (W - w) / d;
  int const dH = (H - h) / d;
  XUnmapWindow(dpy, c->hd0.win);
  /*XUnmapWindow(dpy, c->win);*/
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

  /*XMapWindow(dpy, c->win);*/
  XMapWindow(dpy, c->hd0.win);
}

void
cli_ord_init(Window const win) {
  XSetWindowBorderWidth(dpy, win, 0);
  XMapWindow(dpy, win);
  XSelectInput(dpy, win, KMASK);
}

