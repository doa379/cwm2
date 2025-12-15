#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "arrange.h"

#include "res/cbox_8x8.xbm"
#include "res/cboxf_8x8.xbm"
#include "res/cx_8x8.xbm"
#include "res/cbox_16x16.xbm"
#include "res/cboxf_16x16.xbm"
#include "res/cx_16x16.xbm"
#include "res/cbox_24x24.xbm"
#include "res/cboxf_24x24.xbm"
#include "res/cx_24x24.xbm"

extern Display* dpy;
extern cblk_t mons;
extern cblk_t wks;
extern wk_t* currwk;

extern unsigned ch;
extern unsigned cw;
extern unsigned const bdrw;

static unsigned currmon;
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

void cli_wg_init(void) {
  BTN[MIN] = ch < 20 ? cbox_16x16_bits : cbox_24x24_bits;
  BTN[MAX] = ch < 20 ? cboxf_16x16_bits : cboxf_24x24_bits;
  BTN[CLS] = ch < 20 ? cx_16x16_bits : cx_24x24_bits;
  btnh = btnw = ch < 20 ? 16 : 24;
  btnx = 0.5 * (cw - btnw);
  btny = 0.5 * (ch - btnh);
}

cli_t cli_init(Window const win) {
  /* Init parent */
  wg_t const par = wg_init(DefaultRootWindow(dpy), 
      0, 0, 1, 1, bdrw);
  XReparentWindow(dpy, win, par.win, 0, ch);
  XSetWindowBorderWidth(dpy, win, 0);
  /* Init header */
  wg_t hdr = wg_init(par.win, 0, 0, 1, 1, 0);
  hdr.pixmap = XCreateBitmapFromData(dpy, hdr.win, 
    (char*) stipple_8x8, 8, 8);
  XSetStipple(dpy, hdr.gc[BG], hdr.pixmap);
  XSetFillStyle(dpy, hdr.gc[BG], FillStippled);
  XSetStipple(dpy, hdr.gc[ACT], hdr.pixmap);
  XSetFillStyle(dpy, hdr.gc[ACT], FillStippled);
  /* Init min */
  wg_t min = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  min.pixmap = XCreateBitmapFromData(dpy, min.win, 
    (char const*) BTN[MIN], btnw, btnh);
  wg_gc_setbg(min.gc[BG], BG);
  wg_gc_setbg(min.gc[ACT], ACT);
  /* Init max */
  wg_t max = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  max.pixmap = XCreateBitmapFromData(dpy, max.win, 
    (char const*) BTN[MAX], btnw, btnh);
  wg_gc_setbg(max.gc[BG], BG);
  wg_gc_setbg(max.gc[ACT], ACT);
  /* Init cls */
  wg_t cls = wg_init(hdr.win, 0, 0, btnw, btnh, 0);
  cls.pixmap = XCreateBitmapFromData(dpy, cls.win, 
    (char const*) BTN[CLS], btnw, btnh);
  wg_gc_setbg(cls.gc[BG], BG);
  wg_gc_setbg(cls.gc[ACT], ACT);
  /* Init ico */
  wg_t ico = wg_init(currwk->wg.win, 0, 2 * bdrw, 
      cw - 2 * bdrw, ch - 2 * bdrw, bdrw);

  return (cli_t) {
    .wk = currwk,
    .mon = currmon,
    .win = win,
    .par = par,
    .hdr = hdr,
    .min = min,
    .max = max,
    .cls = cls,
    .ico = ico,
    .mode = RES,
    .fs = 0,
  };
}

void cli_deinit(cli_t* const c) {
  wg_deinit(&c->cls);
  wg_deinit(&c->max);
  wg_deinit(&c->min);
  wg_deinit(&c->hdr);
  wg_deinit(&c->par);
  wg_deinit(&c->ico);
  cblk_unmap(&c->wk->clis, c);
}

cli_t* currwk_cli(Window const win) {
  for (cli_t* c = currwk->clis.beg; c != currwk->clis.end; c++)
    if (c->win == win ||
        c->par.win == win ||
        c->hdr.win == win ||
        c->min.win == win ||
        c->max.win == win ||
        c->cls.win == win ||
        c->shd.win == win ||
        c->ico.win == win)
      return c;
  
  return NULL;
}

cli_t* cli(Window const win) {
  for (wk_t const* wk = wks.beg; wk != wks.end; wk++)
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++)
      if (c->win == win ||
          c->par.win == win ||
          c->hdr.win == win ||
          c->min.win == win ||
          c->max.win == win ||
          c->cls.win == win ||
          c->shd.win == win ||
          c->ico.win == win)
        return c;
  
  return NULL;
}

wg_t* cli_wg(cli_t* const c, Window const win) {
  return c->par.win == win ? &c->par :
    c->hdr.win == win ? &c->hdr :
    c->min.win == win ? &c->min :
    c->max.win == win ? &c->max :
    c->cls.win == win ? &c->cls :
    &c->ico;
}

void cli_wg_focus(cli_t* const c, unsigned const clr) {
  wg_win_setbg(c->par.win, clr);
  wg_win_setbdr(c->par.win, clr);
  wg_win_setbg(c->hdr.win, clr);
  XFillRectangle(dpy, c->hdr.win, c->hdr.gc[clr],
    c->hdr.str.ext + 4 * c->par.bdrw, 0, c->hdr.w, 
      ch - 2 * c->hdr.bdrw);
  wg_str_draw(&c->hdr, clr, 2 * c->par.bdrw);

  {
    wg_t const* wg = &c->min;
    XFillRectangle(dpy, wg->win, wg->gc[clr], 
        wg->x, wg->y, wg->w, wg->h);
    XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc[clr], 
        0, 0, wg->w, wg->h, 0, 0, 1);
  }
  
  {
    wg_t const* wg = &c->max;
    XFillRectangle(dpy, wg->win, wg->gc[clr], 
        wg->x, wg->y, wg->w, wg->h);
    XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc[clr], 
        0, 0, wg->w, wg->h, 0, 0, 1);
  }
  
  {
    wg_t const* wg = &c->cls;
    XFillRectangle(dpy, wg->win, wg->gc[clr], 
        wg->x, wg->y, wg->w, wg->h);
    XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc[clr], 
        0, 0, wg->w, wg->h, 0, 0, 1);
  }
  
  wg_win_setbg(c->ico.win, clr);
  wg_win_setbdr(c->ico.win, clr);
  wg_str_draw(&c->ico, clr, 0);
}

void cli_focus(cli_t* const c) {
  if (currwk->currc) {
    cli_wg_focus(currwk->currc, BG);
  }

  cli_wg_focus(c, ACT);
  currwk->prevc = currwk->currc ? currwk->currc : c;
  currwk->currc = c;
}

void cli_currmon_move(void) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++)
      if (c->mon > mons.size - 1) {
        mon_t const* mon = cblk_itr(&mons, currmon);
        XMoveWindow(dpy, c->par.win, mon->x, mon->y);
        c->mon = currmon;
      }
}

void cli_conf(cli_t* const c, int const w, int const h) {
  if (XResizeWindow(dpy, c->par.win, w, h + ch)) {
    c->par.w0 = c->par.w;
    c->par.w = w;
    c->par.h0 = c->par.h;
    c->par.h = h + ch;
    if (XResizeWindow(dpy, c->hdr.win, w, ch)) {
      c->hdr.w = w;
      c->hdr.h = ch;
      XMoveWindow(dpy, c->hdr.win, 0, 0);
      int const y = 0.5 * (c->hdr.h - btnh);
      XMoveWindow(dpy, c->cls.win, c->hdr.w - btnw, y);
      XMoveWindow(dpy, c->max.win, 
          c->hdr.w - 2 * btnw - bdrw, y);
      XMoveWindow(dpy, c->min.win, 
          c->hdr.w - 3 * btnw - 2 * bdrw, y);
    }

    XResizeWindow(dpy, c->win, w, h);
  }
}

void cli_arrange(cli_t* const c, int const x, 
    int const y) {
  if (XMoveWindow(dpy, c->par.win, x + cw, y + ch)) {
    c->par.x = x + cw;
    c->par.y = y + ch;
  }
}
