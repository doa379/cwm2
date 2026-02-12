#pragma once

typedef struct {
  int x;
  int y;
  int w;
  int h;
  /* BB */
  int x1;
  int y1;
} mon_t;

int mon_mons_init(void);
void mon_mons_deinit(void);
void mon_mons_clear(void);
mon_t* mon_currmon(int const, int const);
void mon_conf(void);
