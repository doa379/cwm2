#pragma once

int sel_init(void);
void sel_deinit(void);
void sel_map(wg_t const*);
void sel_unmap(wg_t const*);
void sel_clear(void);
wg_t* sel_find(wg_t const*);

