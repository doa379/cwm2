#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <signal.h>

#include "root.h"
#include "wm.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "tray.h"
#include "panel.h"
#include "status.h"
#include "sel.h"
#include "arrange.h"
#include "prop.h"
#include "lt.h"
#include "ev.h"

#include "calls.h"

extern Display* dpy;

extern cblk_t wks;
extern wk_t* prevwk;
extern wk_t* currwk;

extern cblk_t mons;
extern mon_t* currmon;

extern wg_t panel;
extern tray_t tray;
extern prop_t prop;

static void
calls_mon_switch(mon_t* const mon) {
  if (mon != currmon) {
    char str[16];
    sprintf(str, "Mon %lu/%lu", 
      cblk_dist(&mons, mon) + 1, mons.size);
    currmon = mon;
    status_mon_str_set(str);
    status_mon_draw(wg_BG);
    panel_arrange(currwk);
 
    XWarpPointer(dpy, None, DefaultRootWindow(dpy), 
      0, 0, 0, 0,
        mon->x + 0.5 * mon->w, mon->y + 0.5 * mon->h);
    XFlush(dpy);
    /*XTestFakeButtonEvent(dpy, 1, False, CurrentTime);*/
    XTestFakeButtonEvent(dpy, 1, False, 0);
    XFlush(dpy);
  }
}

void calls_mon1(void) {
  mon_t* const mon = cblk_itr(&mons, 0);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon2(void) {
  mon_t* const mon = cblk_itr(&mons, 1);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon3(void) {
  mon_t* const mon = cblk_itr(&mons, 2);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon4(void) {
  mon_t* const mon = cblk_itr(&mons, 3);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon5(void) {
  mon_t* const mon = cblk_itr(&mons, 4);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon6(void) {
  mon_t* const mon = cblk_itr(&mons, 5);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon7(void) {
  mon_t* const mon = cblk_itr(&mons, 6);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon8(void) {
  mon_t* const mon = cblk_itr(&mons, 7);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon9(void) {
  mon_t* const mon = cblk_itr(&mons, 8);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon10(void) {
  mon_t* const mon = cblk_itr(&mons, 9);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon11(void) {
  mon_t* const mon = cblk_itr(&mons, 10);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void calls_mon12(void) {
  mon_t* const mon = cblk_itr(&mons, 11);
  if (mon) {
    calls_mon_switch(mon);
  }
}

void
calls_wk0(void) {

}

static void
calls_wk_switch(wk_t* const wk) {
  if (wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk_prev(void) {
  wk_t* const wk = cblk_prev(&wks, currwk);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk_next(void) {
  wk_t* const wk = cblk_next(&wks, currwk);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk_last(void) {
  if (prevwk != currwk) {
    wm_wk_switch(prevwk);
    panel_icos_arrange(currwk);
    panel_arrange(currwk);
  }
}

void
calls_wk1(void) {
  wk_t* const wk = cblk_itr(&wks, 0);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk2(void) {
  wk_t* const wk = cblk_itr(&wks, 1);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk3(void) {
  wk_t* const wk = cblk_itr(&wks, 2);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk4(void) {
  wk_t* const wk = cblk_itr(&wks, 3);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk5(void) {
  wk_t* const wk = cblk_itr(&wks, 4);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk6(void) {
  wk_t* const wk = cblk_itr(&wks, 5);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk7(void) {
  wk_t* const wk = cblk_itr(&wks, 6);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk8(void) {
  wk_t* const wk = cblk_itr(&wks, 7);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk9(void) {
  wk_t* const wk = cblk_itr(&wks, 8);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk10(void) {
  wk_t* const wk = cblk_itr(&wks, 9);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk11(void) {
  wk_t* const wk = cblk_itr(&wks, 10);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk12(void) {
  wk_t* const wk = cblk_itr(&wks, 12);
  if (wk) {
    calls_wk_switch(wk);
  }
}

void
calls_wk_map(void) {
  if (wm_wk_map()) {
    panel_icos_arrange(currwk);
    panel_arrange(currwk);
  }
}

void
calls_wk_unmap(void) {
  if (wm_wk_unmap(currwk) == 0) {
    panel_icos_arrange(currwk);
    panel_arrange(currwk);
  }
}

static void
calls_cli_wk_move(cli_t* const c, wk_t* const wk) {
  if (c && wk && wk != c->wk) {
    wk_t* const currwk = c->wk;
    if (wm_cli_move(c, wk)) {
      wm_ico_enum(wk);
      wm_ico_enum(currwk);
      panel_icos_arrange(currwk);
      panel_arrange(currwk);
    }
  }
}

void
calls_cli_wk_prev_move(void) {
  wk_t* const wk = cblk_prev(&wks, currwk);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk_next_move(void) {
  wk_t* const wk = cblk_next(&wks, currwk);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk1_move(void) {
  wk_t* const wk = cblk_itr(&wks, 0);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk2_move(void) {
  wk_t* const wk = cblk_itr(&wks, 1);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk3_move(void) {
  wk_t* const wk = cblk_itr(&wks, 2);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk4_move(void) {
  wk_t* const wk = cblk_itr(&wks, 3);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk5_move(void) {
  wk_t* const wk = cblk_itr(&wks, 4);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk6_move(void) {
  wk_t* const wk = cblk_itr(&wks, 5);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk7_move(void) {
  wk_t* const wk = cblk_itr(&wks, 6);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk8_move(void) {
  wk_t* const wk = cblk_itr(&wks, 7);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk9_move(void) {
  wk_t* const wk = cblk_itr(&wks, 8);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk10_move(void) {
  wk_t* const wk = cblk_itr(&wks, 9);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk11_move(void) {
  wk_t* const wk = cblk_itr(&wks, 10);
  calls_cli_wk_move(currwk->currc, wk);
}

void
calls_cli_wk12_move(void) {
  wk_t* const wk = cblk_itr(&wks, 11);
  calls_cli_wk_move(currwk->currc, wk);
}

static void
calls_cli_switch(cli_t* const c) {
  if (c != c->wk->currc) {
    XRaiseWindow(dpy, c->par.win);
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli_prev(void) {
  cli_t* const c = cblk_prev(&currwk->clis, currwk->currc);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli_next(void) {
  cli_t* const c = cblk_next(&currwk->clis, currwk->currc);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli_last(void) {
  if (currwk->prevc && currwk->prevc != currwk->currc) {
    wm_cli_switch(currwk->prevc);
    XRaiseWindow(dpy, currwk->currc->par.win);
    panel_icos_arrange(currwk);
  }
}

void
calls_cli0(void) {

}

void
calls_cli1(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 0);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli2(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 1);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli3(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 2);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli4(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 3);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli5(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 4);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli6(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 5);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli7(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 6);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli8(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 7);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_cli9(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 8);
  if (c && c != currwk->currc) {
    calls_cli_switch(c);
  }
}

void
calls_arrange_toggle(void) {
  lt_arrange(currwk, currmon);
}

void
calls_cli_fs_toggle(void) {
  cli_t* const c = currwk->currc;
  if (c) {
    prop_win_fs(c->ker.win);
  }
}

void
calls_cli_mode_toggle(void) {
  cli_t* const c = currwk->currc;
  if (c) {
    if (c->mode == cli_MAX) {
      wm_cli_res(c);
    } else if (c->mode == cli_RES) {
      wm_cli_max(c);
    }
  }
}

void
calls_cli_raise_toggle(void) {
  cli_t* const c = currwk->currc;
  if (c) {
    if (c->mode == cli_MIN) {
      wm_cli_raise(c);
    } else {
      wm_cli_min(c);
    }
  }
}

void
calls_sel_toggle(void) {
  if (currwk->clis.size == 0) {
    return;
  }

  cli_t* c = currwk->currc;
  wg_t* const sel_wg = sel_find(&c->par);
  if (sel_wg == NULL) {
    sel_map(&c->par);
  } else {
    sel_unmap(&c->par);
  }

  c->sel = !c->sel;
  wg_win_bdrclr(c->par.win, c->sel ? wg_SEL :
    c == c->wk->currc ? wg_ACT : wg_BG);
}

void
calls_sel_clear(void) {
  if (currwk->clis.size == 0) {
    return;
  }

  cli_t* c = currwk->clis.front;
  do {
    c->sel = 0;
    wg_win_bdrclr(c->par.win, 
      c == c->wk->currc ? wg_ACT : wg_BG);
    c = cblk_next(&currwk->clis, c);
  } while (c != currwk->clis.front);

  sel_clear();
}

void
calls_quit(void) {
  raise(SIGINT);
}

void
calls_kill(void) {
  cli_t* const c = currwk->currc;
  if (c) {
    prop_win_del(c->ker.win);
  }
}

void
calls_cli_move(void) {
  int x_root;
  int y_root;
  Window const win = root_ptr_query(&x_root, &y_root);
  cli_t* const c = wm_cli(win);
  if (c && (c->mode == cli_RES && c->fs == 0)) {
    XRaiseWindow(dpy, c->par.win);
    wm_cli_translate(c, x_root, y_root);
  }
}

void
calls_cli_resize(void) {
  /*
  int x_root;
  int y_root;
  Window const win = root_ptr_query(&x_root, &y_root);
  cli_t* const c = wm_cli(win);
  */
  cli_t* const c = currwk->currc;
  if (c && (c->mode == cli_RES && c->fs == 0)) {
    XRaiseWindow(dpy, c->par.win);
    wm_cli_resize(c);
  }
}

void
calls_debug(void) {
  wk_t* wk = wks.front;
  do {
    fprintf(stdout, "wk %p (prevc %p, currc %p)\n", 
      (void*) wk, (void*) wk->prevc, (void*) wk->currc);
    cli_t* c = wk->clis.front; 
    do {
      fprintf(stdout, "cli %p\n", (void*) c);
      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front);
    wk = cblk_next(&wks, wk);
  } while (wk != wks.front); 
}
