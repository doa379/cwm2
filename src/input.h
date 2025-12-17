#pragma once

typedef struct {
  int mod;
  int sym;
  void (*call)(void);
  char const* cmd;
} input_t;

void input_btns_deinit(void);
void input_btn_init(input_t const*);
void input_keys_grab(Window const);
void input_btns_grab(Window const);
void input_btns_ungrab(Window const);
void input_key(unsigned const, unsigned const);
void input_btn(unsigned const, unsigned const);
