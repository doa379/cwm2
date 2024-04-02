#pragma once

#include <X11/Xlib.h>
#include <ev.h>

bool init_root(Display*);
void deinit_root();
int modmask();

bool xinerama();
int init_queryscreens();
void deinit_queryscreens();
void query_screen(const int, unsigned*, unsigned*, unsigned*, unsigned*);

void init_windows();
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
void focusin(const Window);
bool send_killmsg(const Window);
bool send_switchwks(const unsigned);
void spawn(const char*);
Window init_shadow(const unsigned, const unsigned);
void destroy_window(const Window);
void cascade(int*, int*, const unsigned, const unsigned);
