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
extern cblk_t mons;
extern wg_t panel;
extern tray_t tray;
extern cblk_t wks;
extern wk_t* prevwk;
extern wk_t* currwk;

void calls_mon0(void) {

}

void calls_mon1(void) {

}

void calls_mon2(void) {

}

void calls_mon3(void) {

}

static void calls_wk_switch(int const n) {
  if (wk_switch(n) == 0) {
    panel_icos_arrange();
    panel_arrange();
  }
}

void calls_wk_prev(void) {
  calls_wk_switch(-2);
}

void calls_wk_next(void) {
  calls_wk_switch(-1);
}

void calls_wk_last(void) {
  size_t const n = cblk_dist(&wks, prevwk);
  calls_wk_switch(n + 1);
}

void calls_wk0(void) {

}

void calls_wk1(void) {
  calls_wk_switch(1);
}

void calls_wk2(void) {
  calls_wk_switch(2);
}

void calls_wk3(void) {
  calls_wk_switch(3);
}

void calls_wk4(void) {
  calls_wk_switch(4);
}

void calls_wk5(void) {
  calls_wk_switch(5);
}

void calls_wk6(void) {
  calls_wk_switch(6);
}

void calls_wk7(void) {
  calls_wk_switch(7);
}

void calls_wk8(void) {
  calls_wk_switch(8);
}

void calls_wk9(void) {
  calls_wk_switch(9);
}

void calls_wk_map(void) {
  if (wk_init()) {
    panel_icos_arrange();
    panel_arrange();
  }
}

void calls_wk_unmap(void) {
  if (wk_unmap(currwk) == 0) {
    size_t const n = cblk_dist(&wks, currwk);
    calls_wk_switch(n + 1);
  }
}

void calls_wk_cli_move_prev(void) {
  if (wm_cli_move(-2) == 0) {
    panel_icos_arrange();
    panel_arrange();
  }
}

void calls_wk_cli_move_next(void) {
  if (wm_cli_move(-1) == 0) {
    panel_icos_arrange();
    panel_arrange();
  }
}

void calls_cli_prev(void) {
  cli_t* const c = cli_switch(-2);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli_next(void) {
  cli_t* const c = cli_switch(-1);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli_last(void) {
  wm_cli_focus(currwk->prevc); 
}

void calls_cli_raise_toggle(void) {
  static int t;
  for (cli_t* c = currwk->clis.beg; 
      c != currwk->clis.end; c++) {
    if (t++ == 0)
      XUnmapWindow(dpy, c->par.win);
    else
      XMapRaised(dpy, c->par.win);
  }

  t %= 2;
}

void calls_cli0(void) {

}

void calls_cli1(void) {
  cli_t* const c = cli_switch(1);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli2(void) {
  cli_t* const c = cli_switch(2);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli3(void) {
  cli_t* const c = cli_switch(3);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli4(void) {
  cli_t* const c = cli_switch(4);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli5(void) {
  cli_t* const c = cli_switch(5);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli6(void) {
  cli_t* const c = cli_switch(6);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli7(void) {
  cli_t* const c = cli_switch(7);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli8(void) {
  cli_t* const c = cli_switch(8);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_cli9(void) {
  cli_t* const c = cli_switch(-9);
  if (c) {
    wm_cli_focus(c);
    panel_icos_arrange();
  }
}

void calls_arrange(void) {
  /* Cycle through global Modes: 
      Grid, Cascade, Floating
  */
}

void calls_cli_mode_toggle(void) {
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
}

void calls_sel_clear(void) {
  arrange_sel_clear();
}

void calls_sel_toggle(void) {
  /*
  wm_cli_t const* c = wm_curr_cli();
  arrange_map_sel(c->parwin);
  */
}

void calls_quit(void) {
  raise(SIGINT);
}

void calls_kill(void) {
  cli_t* const c = currwk->currc;
  if (c) {
    wm_cli_kill(c);
    panel_icos_arrange();
    panel_arrange();
  }
}

/*
void calls_rotate_next(void) {

}

void calls_rotate_prev(void) {

}
*/

void calls_move(void) {

}

void calls_resize(void) {

}

void calls_debug(void) {
  fprintf(stdout, "prevwk %p\n", (void*) prevwk);
  fprintf(stdout, "currwk %p\n", (void*) currwk);
  fprintf(stdout, "prevc %p\n", (void*) currwk->prevc);
  fprintf(stdout, "currc %p\n", (void*) currwk->currc);
  for (wk_t* wk = wks.beg; wk != wks.end; wk++) {
    fprintf(stdout, "wk %p\n", (void*) wk);
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++)
      fprintf(stdout, "cli %p\n", (void*) c);
  }
}
