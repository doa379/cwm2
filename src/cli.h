#pragma once

#include "wg.h"
#include "wk.h"

typedef struct cli_s {
  wk_t* wk;
  unsigned mon;
  Window win;
  wg_t par;
  wg_t hdr;
  wg_t min;
  wg_t max;
  wg_t cls;
  wg_t shd;
  wg_t ico;
  int mode;
  int fs;
} cli_t;

enum mode { MIN, MAX, RES, CLS };

void cli_wg_init(void);
cli_t cli_init(Window const);
void cli_deinit(cli_t* const);
cli_t* currwk_cli(Window const);
cli_t* cli(Window const);
wg_t* cli_wg(cli_t* const, Window const);
void cli_wg_focus(cli_t* const, unsigned const);
void cli_focus(cli_t* const);

void cli_currmon_move(void);

void cli_conf(cli_t* const, int const, int const);
void cli_arrange(cli_t* const, int const, int const);
