#pragma once

#include <X11/Xlib.h>

enum event {
  NOOP,
  MAPNOTIFY,
  UNMAPNOTIFY,
  //CLIENTMESSAGE,
  CONFIGUREROOT,
  CONFIGURENOTIFY,
  MAPREQUEST,
  CONFIGUREREQUEST,
  MOTIONNOTIFY,
  KEYPRESS,
  BTNPRESS,
  ENTERNOTIFY,
  PROPERTYNOTIFY
};
  
enum prop {
  WM_PROTOCOLS,
  WM_NAME,
  WM_DELETE_WINDOW,
  WM_STATE,
  WM_TAKE_FOCUS,
  NET_SUPPORTED,
  NET_WM_STATE,
  NET_WM_NAME,
  NET_WM_WINDOW_OPACITY,
  NET_ACTIVE_WINDOW,
  NET_WM_STATE_FULLSCREEN,
  NET_WM_WINDOW_TYPE,
  NET_WM_WINDOW_TYPE_DIALOG,
  NET_CLIENT_LIST,
  NET_NUMBER_OF_DESKTOPS,
  NET_WM_DESKTOP,
  NET_CURRENT_DESKTOP,
  NET_SHOWING_DESKTOP,
};

typedef union {
	char B[20];
	short S[10];
	long L[5];
} data_t;

typedef struct {
  long DATA[4];
  void* ptr[2];
  //data_t data; 
  //void (*evfn)(const data_t);
  void (*evfn)(const long, const long, const long);
  union {
    const enum event EVENT;
    const enum prop PROP;
  };
} ev_t;

bool init_dpy();
void deinit_dpy();
bool init_root();
void deinit_root();

bool xinerama();
int init_queryscreens();
void deinit_queryscreens();
void query_screen(const int, unsigned*, unsigned*, unsigned*, unsigned*);

void init_event(ev_t*);
void init_msgevent(ev_t*);
void init_events();
ev_t* event();
void intr_event();
int modmask();
void init_windows();
void grab_key(const int, const int);
void ungrab_key(const int, const int);
int keycode2sym(const int);
void grab_btn(const Window, const int, const int);
void ungrab_btn(const Window, const int, const int);
void movewindow(const Window, const int, const int);
void mapwindow(const Window);
void unmapwindow(const Window);
void init_atoms();
void app_clientlist(const Window);
void clear_clientlist();
void del_actwindow(const Window);
void set_nwks(const int);
void set_wks(const int);
void init_ewmh();
void set_bdrcolor(const Window, const size_t);
void set_bdrwidth(const Window, const size_t);
void focusin(const Window);
bool send_killmsg(const Window);
bool send_switchwks(const unsigned);
void spawn(const char*);
void cascade(int*, int*, const unsigned, const unsigned);
Window init_shadow(const unsigned, const unsigned);
void destroy_window(const Window);
void init_drawable();
void deinit_drawable();
unsigned dpywidth();
unsigned dpyheight();
unsigned dpydepth();
GC init_gc();
void deinit_gc(const GC);
void init_print();
void deinit_print();
void draw_wks(const char*, const GC, const size_t, const size_t, 
  const unsigned, unsigned*);
void draw_root(const char*, const GC, const size_t, const size_t, 
  const unsigned, const unsigned, unsigned*);
void draw_client(const char*, const GC, const size_t, const size_t, unsigned*);
