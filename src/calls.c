#include <X11/Xlib.h>
#include <signal.h>
#include <stdio.h>

#include "wm.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "tray.h"
#include "panel.h"
#include "arrange.h"

extern Display* dpy;

extern cblk_t wks;
extern wk_t* prevwk;
extern wk_t* currwk;

extern cblk_t mons;
extern wg_t panel;
extern tray_t tray;

void
calls_wk0(void) {

}

void
calls_wk_prev(void) {
  wk_t* const wk = cblk_prev(&wks, currwk);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk_next(void) {
  wk_t* const wk = cblk_next(&wks, currwk);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
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
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk2(void) {
  wk_t* const wk = cblk_itr(&wks, 1);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk3(void) {
  wk_t* const wk = cblk_itr(&wks, 2);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk4(void) {
  wk_t* const wk = cblk_itr(&wks, 3);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk5(void) {
  wk_t* const wk = cblk_itr(&wks, 4);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk6(void) {
  wk_t* const wk = cblk_itr(&wks, 5);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk7(void) {
  wk_t* const wk = cblk_itr(&wks, 6);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk8(void) {
  wk_t* const wk = cblk_itr(&wks, 7);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk9(void) {
  wk_t* const wk = cblk_itr(&wks, 8);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk10(void) {
  wk_t* const wk = cblk_itr(&wks, 9);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk11(void) {
  wk_t* const wk = cblk_itr(&wks, 10);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
calls_wk12(void) {
  wk_t* const wk = cblk_itr(&wks, 12);
  if (wk && wk != currwk) {
    wm_wk_switch(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
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

void
calls_cli_prev(void) {
  cli_t* const c = cblk_prev(&currwk->clis, currwk->currc);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli_next(void) {
  cli_t* const c = cblk_next(&currwk->clis, currwk->currc);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli_last(void) {
  if (currwk->prevc && currwk->prevc != currwk->currc) {
    wm_cli_switch(currwk->prevc);
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
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli2(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 1);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli3(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 2);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli4(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 3);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli5(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 4);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli6(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 5);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli7(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 6);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli8(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 7);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli9(void) {
  cli_t* const c = cblk_itr(&currwk->clis, 8);
  if (c && c != currwk->currc) {
    wm_cli_switch(c);
    panel_icos_arrange(c->wk);
  }
}

void
calls_cli_raise_toggle(void) {

}

void
calls_grid_arrange(void) {
  if (currwk->clis.size == 0)
    return;

  cli_t* c = currwk->clis.front;
  do {
    arrange_sel_map(&c->par);
    c = cblk_next(&currwk->clis, c);
  } while (c != currwk->clis.front);

  mon_t const* mon = mons.front;
  arrange_sel_tile(mon->w, mon->h);
  do {
    cli_conf(c, c->par.w, c->par.h);
    c = cblk_next(&currwk->clis, c);
  } while (c != currwk->clis.front);
}

void
calls_cli_mode_toggle(void) {
  /*
  cli_t* const c = currwk->currc;
  if (c) {
    ++c->mode; 
    c->mode %= 2;
    wg_t* const wg = &c->par;
    if (c->mode == RES)
      cli_conf(c, wg->w0, wg->h0);
    else if (c->mode == MAX) {
      mon_t const* mon = cblk_itr(&mons, c->mon);
      if (mon == mons.beg)
        cli_conf(c, mon->w - tray.wg.w, 
          mon->h - panel.h);
      else
        cli_conf(c, mon->w, mon->h);
    }
  }
  */
}

void
calls_sel_clear(void) {
  arrange_sel_clear();
}

void
calls_sel_toggle(void) {

}

void
calls_quit(void) {
  raise(SIGINT);
}

void
calls_kill(void) {
  cli_t* const c = currwk->currc;
  if (c) {
    wm_cli_kill(c);
    panel_icos_arrange(currwk);
    panel_arrange(currwk);
  }
}

void
calls_cli_move(void) {
  fprintf(stdout, "Call move on client\n");
  wm_cli_translate(currwk->currc);
}

void
calls_cli_resize(void) {
  fprintf(stdout, "Call resize on client\n");
  wm_cli_resize(currwk->currc);
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
