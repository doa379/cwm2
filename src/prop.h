#pragma once

static long const _NET_WM_STATE_REMOVE = 0;
static long const _NET_WM_STATE_ADD = 1;
static long const _NET_WM_STATE_TOGGLE = 2;

typedef struct {
  Atom utf8string;
  Atom wm_proto;
  Atom wm_state;
  Atom wm_delwin;
  Atom wm_iconame;
  Atom net_supported;
  Atom net_check;
  Atom net_name;
  Atom net_iconame;
  Atom net_clients;
  Atom net_state;
  Atom net_type;
  Atom net_fs;
  Atom net_actwin;
} prop_t;

void prop_init(void);
char const* prop_root(void);
char const* prop_name(Window const);
char const* prop_ico(Window const);
void prop_win_del(Window const);
void prop_win_fs(Window const);
void prop_win_config(Window const, int const, int const,
int const, int const, int const);
char const* prop_atom_name(Atom const);
void prop_state_set(Window const, long const);
