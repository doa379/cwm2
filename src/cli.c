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
extern unsigned const bw;

static unsigned char const stipple_8x8[] = {
  /* Every 3rd bit */
  /* 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00 */
  0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55
};

static long const KMASK =
  PropertyChangeMask;
static long const CLIMASK = 
  EnterWindowMask |
  LeaveWindowMask |
  FocusChangeMask |
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
static unsigned bw2;
static unsigned btnw;
static unsigned btnh;
static unsigned hdrh;
static unsigned btnx;
static unsigned btny;
static unsigned btndx;
static unsigned char* BTN[cli_END];
static wg_t anim;

void
cli_wg_init(void) {
  bw2 = 2 * bw;
  btnh = btnw = font.ch < 24 ? 16 : 24;
  hdrh = font.ch < btnh ? btnh : font.ch;
  btnx = 0.5 * (font.cw - btnw);
  btny = 0.5 * (hdrh - btnh);
  btndx = btnw + bw2;
  
  BTN[cli_MIN] = hdrh < 24 ? 
    cbox_16x16_bits : cbox_24x24_bits;
  BTN[cli_MAX] = hdrh < 24 ? 
    cboxf_16x16_bits : cboxf_24x24_bits;
  BTN[cli_RES] = hdrh < 24 ? 
    cboxff_16x16_bits : cboxff_24x24_bits;
  BTN[cli_CLS] = hdrh < 24 ? 
    cx_16x16_bits : cx_24x24_bits;
  BTN[cli_SIZ] = hdrh < 24 ? 
    siz_16x16_bits : siz_24x24_bits;
  anim = wg_init(DefaultRootWindow(dpy), 1, 1, 0); 
  wg_win_bgclr(anim.win, wg_BG);
}

void
cli_wg_deinit(void) {
  wg_deinit(&anim);
}

cli_t
cli_init(Window const win, wk_t* const wk) {
  /* Init parent */
  wg_t const par = wg_init(DefaultRootWindow(dpy), 
      1, 1, bw);
  wg_win_bgclr(par.win, wg_BG);
  XReparentWindow(dpy, win, par.win, 0, 0);
  XSelectInput(dpy, par.win, CLIMASK);
  XSetWindowBorderWidth(dpy, win, 0);
  XSelectInput(dpy, win, KMASK);
  /* Init special header */
  wg_t hd1 = wg_init(par.win, 1, 1, 0);
  wg_win_bgclr(hd1.win, wg_SEL);
  wg_win_move(&hd1, 0, 0);
  /* Init general header */
  wg_t hd0 = wg_init(par.win, 1, 1, 0);
  hd0.pixmap = XCreateBitmapFromData(dpy, hd0.win, 
    (char*) stipple_8x8, 8, 8);
  XSetStipple(dpy, hd0.gc, hd0.pixmap);
  XSetFillStyle(dpy, hd0.gc, FillStippled);
  XSelectInput(dpy, hd0.win, HDRMASK);
  wg_win_move(&hd0, 0, 0);
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
  XMoveWindow(dpy, cls.win, bw2, btny);
  /* Init siz */
  wg_t siz = wg_init(hd0.win, btnw, btnh, 0);
  siz.pixmap = XCreateBitmapFromData(dpy, siz.win, 
    (char const*) BTN[cli_SIZ], btnw, btnh);
  XSelectInput(dpy, siz.win, BTNMASK);
  /* Init ico */
  wg_t ico = wg_init(wk->wg.win, 
    1.25 * font.cw - bw2, font.ch, bw);
  XSelectInput(dpy, ico.win, BTNMASK);
  /* */
  return (cli_t) {
    .wk = wk,
    .ker = (wg_t) { 
      .win = win,
      .w = 1,
      .h = 1,
    },
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
    .fs = 0,
    .sel = 0,
    .stk = 0,
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
  XSelectInput(dpy, c->ker.win, NoEventMask);
  XReparentWindow(dpy, c->ker.win, DefaultRootWindow(dpy), 
    c->par.x, c->par.y);
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
    if (c->ker.win == win ||
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
cli_clr(cli_t* const c, unsigned const clr) {
  wg_win_bdrclr(c->par.win, c->sel ? wg_SEL : clr);
  unsigned hdx0 = btnw + 2 * bw2;
  /* hd0 offset */
  wg_str_draw(&c->hd0, clr, hdx0);
  XFillRectangle(dpy, c->hd0.win, c->hd0.gc,
    c->hd0.str.ext + 2 + hdx0, 0, c->hd0.w,
      c->hd0.h - 2 * c->hd0.bw);

  wg_pixmap_fill(&c->min, clr);
  wg_pixmap_fill(&c->max, clr);
  wg_pixmap_fill(&c->res, clr);
  wg_pixmap_fill(&c->cls, clr);
  wg_pixmap_fill(&c->siz, clr);
}

void
cli_ico_clr(cli_t* const c, unsigned const clr) {
  wg_win_bgclr(c->ico.win, clr);
  wg_win_bdrclr(c->ico.win, clr);
  wg_str_draw(&c->ico, clr, 0);
}

void
cli_ker_conf(cli_t* const c, int const w, int const h) {
  /* Config elements given kernel dims */
  wg_win_resize(&c->par, w, h + hdrh);
  wg_win_resize(&c->hd0, c->par.w, hdrh);
  wg_win_resize(&c->hd1, c->hd0.w, c->hd0.h);
  /* Max btn */
  XUnmapWindow(dpy, c->max.win);
  /* Res btn */
  XUnmapWindow(dpy, c->res.win);
  /* Res, Max selection */
  if (c->mode == cli_MAX) {
    XMapWindow(dpy, c->res.win);
    XUnmapWindow(dpy, c->siz.win);
    XMoveWindow(dpy, c->res.win, c->par.w - 1 * btndx, btny);
    /* Min btn */
    XMoveWindow(dpy, c->min.win, c->par.w - 2 * btndx, btny);

  } else {
    XMapWindow(dpy, c->max.win);
    XMoveWindow(dpy, c->max.win, c->par.w - 2 * btndx, btny);
    XMapWindow(dpy, c->siz.win);
    /* Min btn */
    XMoveWindow(dpy, c->min.win, c->par.w - 3 * btndx, btny);
  }
  /* Resize btn */
  XMoveWindow(dpy, c->siz.win, c->par.w - 1 * btndx, btny);
  /* Kernel */
  wg_win_moveresize(&c->ker, 0, c->hd0.h, w, h);
}

void
cli_par_conf(cli_t* const c, int const w, int const h, 
  int const bw) {
  /* Config client given parent dims */
  cli_ker_conf(c, w - 2 * bw, h - c->hd0.h - 2 * bw);
}

void
cli_move(cli_t* const c, int const x, int const y, 
int const W, int const H) {
  /* Constraint (W, H) */
  int const cw = c->par.w + bw2;
  int const nextx = x + cw > W ? W - cw : x;
  int const ch = c->par.h + bw2;
  int const nexty = y + ch > H ? H - ch : y;
  wg_win_move(&c->par, nextx, nexty);
  /* TODO want to store rel. coords */
}

void
cli_ker_resize(cli_t* const c, int const w, int const h,
int const W, int const H) {
  /* Constraint (W, H) */
  int const cw = w + bw2 > W ? W - bw2 : w;
  int const dh = hdrh + bw2;
  int const ch = h + dh > H ? H - dh : h;
  cli_ker_conf(c, cw, ch);
}

void
cli_par_resize(cli_t* const c, int const w, int const h,
int const W, int const H) {
  /* Constraint (W, H) */
  int const cw = w > W ? W - bw2 : w;
  int const ch = h > H ? H - bw2 : h;
  cli_par_conf(c, cw, ch, c->par.bw);
}

void
cli_anim(int const x, int const y, 
int const w, int const h, int const X, int const Y, 
int const W, int const H, int const d) {
  XMapRaised(dpy, anim.win);
  int const dX = (X - x) / d;
  int const dY = (Y - y) / d;
  int const dW = (W - w) / d;
  int const dH = (H - h) / d;
  for (int n = 0; n < d; n++) {
    int const nextX = x + dX * n;
    int const nextY = y + dY * n;
    int const nextW = w + dW * n;
    int const nextH = h + dH * n;
    XMoveResizeWindow(dpy, anim.win, nextX, nextY, 
      nextW, nextH);
    XFlush(dpy);
    usleep(1200);
  }

  XUnmapWindow(dpy, anim.win);
}

void
cli_switch_anim(cli_t* const c, int const d) {
  for (unsigned i = 0; i < d; i++) {
    usleep(5000);
    cli_clr(c, wg_BG);
    cli_ico_clr(c, wg_BG);
    XFlush(dpy);
    usleep(5000);
    cli_clr(c, wg_ACT);
    cli_ico_clr(c, wg_ACT);
    XFlush(dpy);
  }
}

void
cli_del_anim(cli_t* const c, int const d) {
  cli_anim(c->par.x, c->par.y, c->par.w, c->par.h,
    c->par.x + 0.5 * c->par.w, c->par.y + 0.5 * c->par.h,
      1, 1, d);
}

void
cli_hd1_draw(cli_t* const c, int const x, int const y) {
  char str[16];
  snprintf(str, sizeof str, "(%d, %d)", x, y);
  wg_str_set(&c->hd1, str);
  unsigned hdx0 = 2 * c->par.bw;
  wg_str_draw(&c->hd1, wg_SEL, hdx0);
}

void
cli_ord_init(Window const win) {
  XSetWindowBorderWidth(dpy, win, 0);
  XMapWindow(dpy, win);
  XSelectInput(dpy, win, KMASK);
}

