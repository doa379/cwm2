#pragma once

#include "wg.h"
#include "wk.h"
#include "arrange.h"

typedef struct cli_s {
  wk_t* wk;
  wg_t ker;
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
  arrange_t fl;
} cli_t;

enum cli_mode { 
  cli_MIN, 
  cli_MAX, 
  cli_RES, 
  cli_CLS, 
  cli_SIZ,
  /* Placeholder */
  cli_END
};

void cli_wg_init(void);
void cli_wg_deinit(void);
cli_t cli_init(Window const, wk_t* const);
void cli_deinit(cli_t* const);
cli_t* cli(Window const, wk_t* const);
wg_t* cli_wg(cli_t* const, Window const);
void cli_clr(cli_t* const, unsigned const);
void cli_ico_clr(cli_t* const, unsigned const);
void cli_ker_conf(cli_t* const, int const, int const);
void cli_par_conf(cli_t* const, int const, int const);
void cli_move(cli_t* const, int const, int const, int const,
int const);
void cli_ker_resize(cli_t* const, int const, int const, 
int const, int const);
void cli_par_resize(cli_t* const, int const, int const,
int const, int const);
void cli_min(cli_t* const, int const, int const);
void cli_res(cli_t* const, int const, int const, 
int const, int const);
void cli_raise(cli_t* const, int const, int const,
int const, int const);
void cli_max(cli_t* const, int const, int const,
int const, int const);
void cli_fs(cli_t* const, int const, int const, int const, 
int const);
void cli_switch_anim(cli_t* const, int const);
void cli_del_anim(cli_t* const, int const);
void cli_hd1_draw(cli_t* const, int const, int const);
void cli_ord_init(Window const);
