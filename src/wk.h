#pragma once

#include "wg.h"
#include "cblk.h"

typedef struct {
  cblk_t clis;
  struct cli_s* prevc;
  struct cli_s* currc;
  wg_t wg;
} wk_t;

wk_t wk_init(void);
void wk_deinit(wk_t* const);
void wk_wg_focus(wk_t* const, unsigned const);
