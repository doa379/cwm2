#pragma once

#include "wg.h"
#include "wk.h"

typedef struct cli_s {
  wk_t* wk;
  /* Kernel */
  Window win;
  int w;
  int h;
  /* Cli (par) org */
  int x0;
  int y0;
  /* Widgets */
  wg_t par;
  /* General */
  wg_t hd0;
  /* Special */
  wg_t hd1;
  wg_t min;
  wg_t max;
  wg_t res;
  wg_t cls;
  wg_t siz;
  wg_t ico;
  char strico[4];
  int mode;
  int fs;
  int sel;
} cli_t;

enum cli_mode { 
  cli_MIN, 
  cli_MAX, 
  cli_RES, 
  cli_CLS, 
  cli_SIZ,
};

void cli_wg_init(void);
cli_t cli_init(Window const, wk_t* const);
void cli_deinit(cli_t* const);
cli_t* cli(Window const, wk_t* const);
wg_t* cli_wg(cli_t* const, Window const);
void cli_wg_focus(cli_t* const, unsigned const);
void cli_conf(cli_t* const, int const, int const);
void cli_move(cli_t* const, int const, int const, int const,
int const);
void cli_resize(cli_t* const, int const, int const, 
int const, int const, int const);
void cli_min(cli_t* const, int const, int const);
void cli_res(cli_t* const, int const, int const, 
int const, int const);
void cli_raise(cli_t* const, int const, int const,
int const, int const);
void cli_max(cli_t* const, int const, int const,
int const, int const);
void cli_fs(cli_t* const, int const, int const, int const, 
int const);
void cli_anim(cli_t* const, int const, int const, int const, 
int const, int const, int const, int const, int const, 
int const);
void cli_switch_anim(cli_t* const, int const);
void cli_del_anim(cli_t* const, int const);
void cli_ord_init(Window const);
