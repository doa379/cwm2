#pragma once

#include "cli.h"

int wm_init(unsigned const);
void wm_deinit(void);
wk_t* wm_wk(Window const);
wk_t* wm_wk_map(void);
void wm_wk_deinit(wk_t* const);
int wm_wk_unmap(wk_t* const);
void wm_wk_unfocus(wk_t* const);
void wm_wk_focus(wk_t* const);
cli_t* wm_cli_map(wk_t* const, Window const);
void wm_cli_unmap(cli_t* const);
cli_t* wm_cli(Window const);
void wm_cli_unfocus(cli_t* const);
void wm_cli_focus(cli_t* const);
cli_t* wm_cli_move(cli_t* const, wk_t* const);
void wm_cli_del(cli_t* const);
void wm_cli_translate(cli_t* const, int const, int const);
void wm_cli_resize(cli_t* const);
void wm_cli_kill(cli_t* const);
void wm_cli_switch(cli_t* const);
void wm_wk_switch(wk_t* const);
void wm_cli_ker_conf(cli_t* const, int const, int const);
void wm_cli_arrange(cli_t* const, int const, int const);
void wm_cli_currmon_move(void);
void wm_cli_min(cli_t* const);
void wm_cli_max(cli_t* const);
void wm_cli_fs(cli_t* const);
void wm_cli_res(cli_t* const);
void wm_cli_raise(cli_t* const);
void wm_cli_ico_enum(cli_t* const);
void wm_ico_enum(wk_t* const);
void wm_tray_cli_map(cli_t* const);
void wm_tray_cli_unmap(wg_t* const);
Window* wm_ord_map(Window const);
void wm_ord_unmap(Window* const);
Window* wm_ord(Window const);
void wm_ord_conf(Window const, int const, int const);
void wm_ord_arrange(Window const, int const, int const);
