#pragma once

#include "wg.h"
#include "cblk.h"

typedef struct {
  cblk_t clis;
  struct cli_s* prevc;
  struct cli_s* currc;
  wg_t wg;
} wk_t;

void wk_wg_unfocus(wg_t* const);
void wk_wg_focus(wg_t* const);
wk_t* wk_init(void);
void wk_deinit(wk_t*);
int wk_unmap(void);
int wk_switch(int const);
