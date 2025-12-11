#pragma once

#include "cli.h"

int wm_init(unsigned const);
void wm_deinit(void);
void wm_cli_focus(cli_t* const);
int wm_cli_move(int const);
