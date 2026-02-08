#include "cli.h"
#include "wk.h"
#include "mon.h"
#include "arrange.h"

#include "sel.h"

extern cblk_t sel;
extern unsigned const hgap;
extern unsigned const vgap;

static unsigned const casc_hgap = 16;
static unsigned const casc_vgap = 16;

enum lt { 
  lt_TILE,
  lt_CASC,
  lt_MONO,
  lt_FLOAT,
  /* Placeholder */
  lt_END
};

static void
lt_scheme_arrange(wk_t* const wk, mon_t const* mon) {
  static int lt;
  if (lt == lt_TILE) {
    wg_t** wg = sel.front;
    size_t n = 0;
    do {
      arrange_t a = arrange_tile(n, sel.size, hgap, vgap,
        mon->w, mon->h);
      cli_t* const c = cli((*wg)->win, wk);
      wg_win_move(&c->par, a.x, a.y);
      cli_par_conf(c, a.w, a.h);
      n++;
      wg = cblk_next(&sel, wg);
    } while (wg != sel.front);


  } else if (lt == lt_CASC) {
    wg_t** wg = sel.front;
    size_t n = 0;
    do {
      arrange_t a = arrange_casc(n, casc_hgap, casc_vgap);
      cli_t* const c = cli((*wg)->win, wk);
      cli_ker_resize(c, c->fl.w, c->fl.h,
        mon->w, mon->h);
      cli_move(c, a.x, a.y, mon->w, mon->h);
      n++;
      wg = cblk_next(&sel, wg);
    } while (wg != sel.front);


  } else if (lt == lt_MONO) {
    wg_t** wg = sel.front;
    size_t n = 0;
    do {
      arrange_t a = arrange_mono(n, sel.size, casc_hgap, 
        casc_vgap, mon->w, mon->h);
      cli_t* const c = cli((*wg)->win, wk);
      cli_par_resize(c, a.w, a.h,
        mon->w, mon->h);
      cli_move(c, a.x, a.y, mon->w, mon->h);
      n++;
      wg = cblk_next(&sel, wg);
    } while (wg != sel.front);
    
    
  } else if (lt == lt_FLOAT) {
    cli_t* c = wk->clis.front;
    do {
      wg_win_move(&c->par, c->fl.x, c->fl.y);
      cli_ker_conf(c, c->fl.w, c->fl.h);
      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front);
  }

  lt++;
  lt %= lt_END;
}

void
lt_arrange(wk_t* const wk, mon_t* const mon) {
  if (sel.size == 0 && wk->clis.size > 1) {
    /* All */
    cli_t* c = wk->clis.front;
    do {
      sel_map(&c->par);
      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front);
    
    lt_scheme_arrange(wk, mon);
    sel_clear();
  } else if (sel.size > 1) {
    lt_scheme_arrange(wk, mon);
  }
}
