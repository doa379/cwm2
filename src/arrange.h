#pragma once

#include "wg.h"

typedef struct {
  int x;
  int y;
  int w;
  int h;
} arrange_t;

int arrange_init(void);
void arrange_deinit(void);
void arrange_sel_map(wg_t const*);
void arrange_sel_clear(void);
void arrange_sel_adj(int const);

arrange_t arrange_tile(size_t const, size_t const, 
  unsigned const, unsigned const, unsigned const, 
  unsigned const);
arrange_t arrange_casc(size_t const, unsigned const, 
unsigned const);
arrange_t arrange_mono(size_t const, size_t const, 
unsigned const, unsigned const, unsigned const, 
unsigned const);
