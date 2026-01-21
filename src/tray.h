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
void tray_mascot_conf(void);
wg_t* tray_cli(Window const);
unsigned tray_cli_vd(void);
void tray_cli_map(wg_t* const);
void tray_cli_unmap(wg_t* const);
