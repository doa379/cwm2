#pragma once

#include <X11/Xlib.h>
#include <util.h>

typedef struct {
  Window w;
  pair_t pos;
  pair_t size;
  GC gc;
  // 32B
  int sel;
  int ft;
  int m0;
  int m1;
  int m2;
  int m3;
  int m4;
  int m5;
} client_t;

typedef struct {
  pair_t size;
} monitor_t;

bool init_wm(Display*, const Window);
void deinit_wm(Display*, const Window);
void configure_root(Display*, const Window, const int, const int);
void map(Display*, const Window, const Window);
void key(Display*, const Window, const int, const int);
static void focus(Display*, const Window, client_t*);
static void quit(Display*, const Window);
static void kill(Display*, const Window);
static client_t* find_client(const Window);
static void prev(Display*, const Window);
static void next(Display*, const Window);
