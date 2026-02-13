#pragma once

#include "wg.h"

typedef struct {
  int x;
  int y;
  int w;
  int h;
} arrange_t;

arrange_t arrange_tile(size_t const, size_t const, 
unsigned const, unsigned const, unsigned const, 
unsigned const);
arrange_t arrange_casc(size_t const, unsigned const, 
unsigned const);
arrange_t arrange_mono(size_t const, size_t const, 
unsigned const, unsigned const, unsigned const, 
unsigned const);
