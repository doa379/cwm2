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
void deinit_wm(Display*, const Window);
void key(Display*, const Window, const int, const int);
void map(Display*, const Window, const Window);
static void focus(Display*, const Window, client_t*);
static void quit(Display*, const Window);
static void kill(Display*, const Window);
static void prev(Display*, const Window);
static void next(Display*, const Window);
