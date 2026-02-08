#pragma once

#include <X11/Xlib.h>

typedef struct {
  int mod;
  int sym;
  void (*call)(void);
  char* cmd;
} input_t;

void input_keys_grab(Window const);
void input_btns_grab(Window const);
void input_btns_ungrab(Window const);
input_t const* input_key(unsigned const, unsigned const);
input_t const* input_btn(unsigned const, unsigned const);
