#pragma once

typedef struct {
  int mod;
  int sym;
  void (*call)(void);
  char const* cmd;
} input_t;

input_t** input_keys_init(unsigned const);
void input_keys_deinit(void);
input_t** input_key_init(input_t const*);
input_t** input_btns_init(unsigned const);
void input_btns_deinit(void);
void input_btn_init(input_t const*);
void input_keys_grab(long const);
void input_btns_grab(long const);
void input_btns_ungrab(long const);
void input_key(unsigned const, unsigned const);
void input_btn(unsigned const, unsigned const);
