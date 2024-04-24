#pragma once

#include <X11/Xlib.h>
#include <ev.h>

bool init_root(Display*);
void deinit_root();
int modmask();

Window* init_querytree(unsigned*);
void deinit_querytree(Window*);

void grab_key(const int, const int);
void ungrab_key(const int, const int);
void grab_btn(const Window, const int, const int);
void ungrab_btn(const Window, const int, const int);
void warp_pointer(const unsigned, const unsigned);
void movewindow(const Window, const int, const int);
void mapwindow(const Window);
void unmapwindow(const Window);
void set_bdrcolor(const Window, const size_t);
void set_bdrwidth(const Window, const size_t);
bool wa_size(int*, int*, const Window);
Window par(const Window, const int, const int, const int, const int,
  const int, const size_t, const size_t);
void reparent(const Window, const Window);
void focusin(const Window);
bool send_killmsg(const Window);
bool send_switchwks(const unsigned);
void spawn(const char*);
Window init_window(const unsigned, const unsigned);
void deinit_window(const Window);

unsigned dpywidth();
unsigned dpyheight();
bool xinerama();
int init_queryscreens();
void deinit_queryscreens();
void query_screen(const int, unsigned*, unsigned*, unsigned*, unsigned*);
