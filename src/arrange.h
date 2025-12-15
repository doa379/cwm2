#pragma once

int arrange_init(void);
void arrange_deinit(void);
void arrange_sel_map(wg_t const*);
void arrange_sel_clear(void);
void arrange_sel_adj(int const);
void arrange_sel_tile(unsigned const, unsigned const);
void arrange_sel_casc(unsigned const, unsigned const);
