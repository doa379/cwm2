#pragma once

#include "wg.h"
#include "cblk.h"
#include "cli.h"

typedef struct {
  cblk_t clis;
  wg_t wg;
} tray_t;

int tray_init(void);
void tray_deinit(void);
void tray_conf(void);
void tray_cli_map(cli_t* const);
