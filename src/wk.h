#pragma once

#include "wg.h"
#include "cblk.h"

typedef struct {
  cblk_t clis;
  struct cli_s* prevc;
  struct cli_s* currc;
  wg_t wg;
} wk_t;

wk_t* wk_init(void);
void wk_deinit(wk_t*);
void wk_wg_focus(wg_t* const, unsigned const);
int wk_unmap(wk_t* const);
int wk_switch(int const);
