#pragma once

#include <X11/Xlib.h>
#include <util.h>

typedef struct {
  Window w;
  pair_t pos;
  pair_t size;
  GC gc;
  int sel;
  int ft;
  int misc;
} client_t;

bool init_wm(Display*, const Window);
void deinit_wm();
void key(const int, const int);
void map(const Window);
static void init_windows();
static void focus(client_t*);
static void quit();
static void kill();
