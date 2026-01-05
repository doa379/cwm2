#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "font.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "arrange.h"

#include "res/cbox_16x16.xbm"
#include "res/cboxf_16x16.xbm"
#include "res/cboxff_16x16.xbm"
#include "res/cx_16x16.xbm"
#include "res/cbox_24x24.xbm"
#include "res/cboxf_24x24.xbm"
#include "res/cboxff_24x24.xbm"
#include "res/cx_24x24.xbm"

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
static unsigned char* BTN[4];

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
  btnh = btnw = font.ch < 20 ? 16 : 24;
  btnx = 0.5 * (font.cw - btnw);
  btny = 0.5 * (font.ch - btnh);
}

cli_t
cli_init(Window const win, wk_t* const wk, mon_t* const mon,
int const x, int const y, int const w, int const h) {
  /* Init parent */
  wg_t const par = wg_init(DefaultRootWindow(dpy), 
      x, y, w, h, bdrw);
  XReparentWindow(dpy, win, par.win, 0, font.ch);
  XSetWindowBorderWidth(dpy, win, 0);
  /* Init header */
  wg_t hdr = wg_init(par.win, 0, 0, 1, 1, 0);
  hdr.pixmap = XCreateBitmapFromData(dpy, hdr.win, 
    (char*) stipple_8x8, 8, 8);
  XSetStipple(dpy, hdr.gc, hdr.pixmap);
  XSetFillStyle(dpy, hdr.gc, FillStippled);
  /* Init min */
  wg_t min = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  min.pixmap = XCreateBitmapFromData(dpy, min.win, 
    (char const*) BTN[MIN], btnw, btnh);
  /* Init max */
  wg_t max = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  max.pixmap = XCreateBitmapFromData(dpy, max.win, 
    (char const*) BTN[MAX], btnw, btnh);
  /* Init res */
  wg_t res = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  res.pixmap = XCreateBitmapFromData(dpy, res.win, 
    (char const*) BTN[RES], btnw, btnh);
  /* Init cls */
  wg_t cls = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  cls.pixmap = XCreateBitmapFromData(dpy, cls.win, 
    (char const*) BTN[CLS], btnw, btnh);
  /* Init ico */
  wg_t ico = wg_init(wk->wg.win, 0, 2 * bdrw, 
      font.cw - 2 * bdrw, font.ch - 2 * bdrw, bdrw);

  return (cli_t) {
    .wk = wk,
    .mon = mon,
    .win = win,
    .par = par,
    .hdr = hdr,
    .min = min,
    .max = max,
    .res = res,
    .cls = cls,
    .ico = ico,
    .mode = RES,
  };
}

void
cli_deinit(cli_t* const c) {
  wg_deinit(&c->cls);
  wg_deinit(&c->res);
  wg_deinit(&c->max);
  wg_deinit(&c->min);
  wg_deinit(&c->hdr);
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
        c->hdr.win == win ||
        c->min.win == win ||
        c->max.win == win ||
        c->res.win == win ||
        c->cls.win == win ||
        c->shd.win == win ||
        c->ico.win == win)
      return c;

    c = cblk_next(&wk->clis, c);
  } while (c != wk->clis.front); 
  
  return NULL;
}

wg_t*
cli_wg(cli_t* const c, Window const win) {
  return c->par.win == win ? &c->par :
    c->hdr.win == win ? &c->hdr :
    c->min.win == win ? &c->min :
    c->max.win == win ? &c->max :
    c->res.win == win ? &c->res :
    c->cls.win == win ? &c->cls :
    &c->ico;
}

void
cli_wg_focus(cli_t* const c, unsigned const clr) {
  wg_win_bgset(c->par.win, clr);
  wg_win_bdrset(c->par.win, clr);
  wg_win_bgset(c->hdr.win, clr);
  wg_gc_bgset(c->hdr.gc, clr);
  XFillRectangle(dpy, c->hdr.win, c->hdr.gc,
    c->hdr.str.ext + 4 * c->par.bdrw, 0, c->hdr.w, 
      font.ch - 2 * c->hdr.bdrw);
  wg_str_draw(&c->hdr, clr, 2 * c->par.bdrw);
  wg_pixmap_fill(&c->min, clr);
  wg_pixmap_fill(&c->max, clr);
  wg_pixmap_fill(&c->res, clr);
  wg_pixmap_fill(&c->cls, clr);
  wg_win_bgset(c->ico.win, clr);
  wg_win_bdrset(c->ico.win, clr);
  wg_str_draw(&c->ico, clr, 0);
}

void
cli_conf(cli_t* const c, int const w, int const h) {
  if (wg_win_resize(&c->par, w, h + font.ch) == 0) {
    if (wg_win_resize(&c->hdr, w, font.ch) == 0) {
      XMoveWindow(dpy, c->hdr.win, 0, 0);
      int const y = 0.5 * (c->hdr.h - btnh);
      XMoveWindow(dpy, c->cls.win, c->hdr.w - btnw, y);
      
      XSelectInput(dpy, c->max.win, 0);
      XUnmapWindow(dpy, c->max.win);
      XSelectInput(dpy, c->max.win, c->max.mask);

      XSelectInput(dpy, c->res.win, 0);
      XUnmapWindow(dpy, c->res.win);
      XSelectInput(dpy, c->res.win, c->res.mask);

      wg_t* const wg = c->mode == MAX ? &c->res : &c->max;
      XMoveWindow(dpy, wg->win, 
          c->hdr.w - 2 * btnw - bdrw, y);

      XSelectInput(dpy, wg->win, 0);
      XMapWindow(dpy, wg->win);
      XSelectInput(dpy, wg->win, wg->mask);
      
      XMoveWindow(dpy, c->min.win, 
          c->hdr.w - 3 * btnw - 2 * bdrw, y);
    }

    XResizeWindow(dpy, c->win, w, h);
  }
}

void
cli_arrange(cli_t* const c, int const x, 
    int const y) {
  wg_win_move(&c->par, x + font.cw, y + font.ch);
}
