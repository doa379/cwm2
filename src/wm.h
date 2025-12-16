#pragma once

#include "cli.h"

int wm_init(unsigned const);
void wm_deinit(void);
cli_t* wm_cli_map(Window const, int const, int const);
void wm_cli_focus(cli_t* const);
int wm_cli_move(wk_t* const);
void wm_cli_kill(cli_t* const);
void wm_cli_translate(cli_t* const);
void wm_cli_resize(cli_t* const);
