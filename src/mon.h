#pragma once

typedef struct {
  int x;
  int y;
  int w;
  int h;
} mon_t;

int mon_mons_init(unsigned const);
void mon_mons_deinit(void);
void mon_mons_clear(void);
unsigned mon_currmon(int const, int const);
void mon_conf(void);
