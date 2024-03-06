#pragma once

#include <X11/Xlib.h>
#include <ev.h>

bool init_dpy();
void deinit_dpy();
bool init_root();
void deinit_root();
void init_event(ev_t*);
void init_events();
ev_t* event();
int modmask();
void init_windows();
void grab_key(const int, const int);
void ungrab_key(const int, const int);
int keycode2sym(const int);
void grab_btn(const Window, const int, const int);
void ungrab_btn(const Window, const int, const int);
void movewindow(const Window, const int, const int);
void mapwindow(const Window);
void init_atoms();
void app_clientlist(const Window);
void clear_clientlist();
void del_actwindow(const Window);
void init_ewmh();
void set_bdrcolor(const Window, const size_t);
void set_bdrwidth(const Window, const size_t);
void focusin(const Window);
void send_killmsg(const Window);
void spawn(const char*);
void init_panel();
void deinit_panel();
GC init_gc();
void deinit_gc(const GC);
void draw_root(const char*);
