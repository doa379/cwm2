#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <Xlib.h>
#include <draw.h>
#include <dbus.h>
#include <../config.h>

typedef struct client_s {
  Window w;
  Window shadow;
  unsigned posx;
  unsigned posy;
  unsigned sizex;
  unsigned sizey;
  GC gc;
  unsigned mode;
  int sel;
  int ft;
  int pad[2];
} client_t;

typedef struct {
  blk_t clients;
  client_t* client[2]; // Prev, Curr
  unsigned n;
} wks_t;

typedef struct {
  unsigned posx;
  unsigned posy;
  unsigned sizex;
  unsigned sizey;
} monitor_t;

static const size_t KBDLEN = { sizeof KBD / sizeof KBD[0] };
static const size_t BTNLEN = { sizeof BTN / sizeof BTN[0] };
static blk_t wks;
static blk_t monitors;
static wks_t* wk[2]; // Prev, Curr
static monitor_t* monitor;
static void (*CALLFN[])() = {
  [WKS0] = wks0,
  [WKS1] = wks1,
  [WKS2] = wks2,
  [WKS3] = wks3,
  [WKS4] = wks4,
  [WKS5] = wks5,
  [WKS6] = wks6,
  [WKS7] = wks7,
  [WKS8] = wks8,
  [WKS9] = wks9,
  [MON0] = mon0,
  [MON1] = mon1,
  [MON2] = mon2,
  [MON3] = mon3,
  [MON4] = mon4,
  [MON5] = mon5,
  [MON6] = mon6,
  [MON7] = mon7,
  [MON8] = mon8,
  [MON9] = mon9,
  [KILL] = kill_curr,
  [TOGGLEMODE] = mode,
  [PREVCLI] = prev,
  [NEXTCLI] = next,
  [QUIT] = quit,
};

static GC statusgc;
static GC wksgc;

// Inits
bool init_wks() {
  wks = init_blk(sizeof(wks_t), NWKS);
  if (beg(&wks) == NULL) 
    return false;

  for (int i = { 0 }; i < NWKS; i++) {
    wks_t w = { 
      .clients = init_blk(sizeof(client_t), 2),
      .n = i 
    };

    if (!beg(&w.clients))
      break;

    map_dev(&wks, &w);
  }

  return true;
}

void deinit_wks() {
  for (wks_t* w = { beg(&wks) }; w != wks.end; w++)
    deinit_blk(&w->clients);
    
  deinit_blk(&wks);
}

bool init_monitors() {
  fprintf(stdout, "Dpysize |%d, %d|\n", dpywidth(), dpyheight());
  // Initial reserve is working correctly
  monitors = init_blk(sizeof(monitor_t), 2);
  if (beg(&monitors) == NULL)
    return false;
  else if (xinerama()) {
    const int N = { init_queryscreens() };
    for (int i = { 0 }; i < N; i++) {
      monitor_t m;
      query_screen(i, &m.posx, &m.posy, &m.sizex, &m.sizey);
      fprintf(stdout, "Mon %d (%d, %d) |%d, %d|\n", 
        i, m.posx, m.posy, m.sizex, m.sizey);
      map_dev(&monitors, &m);
    }

    deinit_queryscreens();
  } else {
    monitor_t m = { .posx = 0, .posy = 0, 
      .sizex = dpywidth(), .sizey = dpyheight()
    };
    
    map_dev(&monitors, &m);
  }
  
  return (monitor = beg(&monitors));
}

void deinit_monitors() {
  deinit_blk(&monitors);
}

void init_wm() {
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < KBDLEN; i++) {
    const int MOD = { KBD[i].mod & MODMASK };
    const int KEY = { KBD[i].key };
    grab_key(MOD, KEY);
  }

  setprop_nwks(wks.size);
  setprop_wks(0);
  wk[0] = wk[1] = beg(&wks);
  clrprop_clientlist();
  unsigned n = { 0 };
  Window* W = { init_querytree(&n) };
  for (unsigned i = { 0 }; i < n; i++) {
    int w = { 0 };
    int h = { 0 };
    if (wa_size(&w, &h, W[i]))
      maprequest(W[i], w, h);
  }

  if (W)
    deinit_querytree(W);

  statusgc = init_gc();
  wksgc = init_gc();
  fprintf(stdout, "Init %s\n", WMNAME);
}

void deinit_wm() {
  deinit_gc(wksgc);
  deinit_gc(statusgc);
  // Restore X11 states to defaults
  for (wks_t* w = { beg(&wks) }; w != wks.end; w++)
    for (client_t* c = { beg(&w->clients) }; c != w->clients.end; c++) {
      set_bdrwidth(c->w, 0);
      mapwindow(c->w);
      deinit_window(c->shadow);
    }

  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < KBDLEN; i++) {
    const int MOD = { KBD[i].mod & MODMASK };
    const int KEY = { KBD[i].key };
    ungrab_key(MOD, KEY);
  }
  
  fprintf(stdout, "Deinit %s\n", WMNAME);
}

// Event calls

void noop(const long, const long, const long) {

}

void mapnotify(const long, const long, const long) {

}

void unmapnotify(const long W, const long, const long) {
  // Validate client
  if (!wk[1]->client[1] || wk[1]->client[1]->w != W)
    return;
  
  wk[1]->client[1] = prev_client(wk[1]->client[1], true);
  clrprop_clientlist();
  client_t* c;
  for (client_t* client = { beg(&wk[1]->clients) }; 
      client != wk[1]->clients.end; client++) {
    if (client->w == W)
      c = client;
    else 
      addprop_clientlist(client->w);
  }
  // No fail  
  //destroy_window(c->shadow);
  deinit_gc(c->gc);
  unmap_dev(&wk[1]->clients, c);
  wk[1]->client[0] = wk[1]->clients.size == 0 ? NULL : wk[1]->client[0];
  wk[1]->client[1] = wk[1]->clients.size == 0 ? NULL : wk[1]->client[1];
}
/*
void clientmessage(const long W, const long, const long) {

}
*/

void configureroot(const long WIDTH, const long HEIGHT, const long) {
  fprintf(stdout, "Config root %ld, %ld\n", WIDTH, HEIGHT);

}

void configurenotify(const long W, const long WIDTH, const long HEIGHT) {

}

void maprequest(const long W, const long WIDTH, const long HEIGHT) {
  map(W);
  // Set origin
  int x = { wk[1]->client[1] && wk[1]->client[1]->posx < monitor->posx ? 
    wk[1]->client[1]->posx : monitor->posx };
  int y = { wk[1]->client[1] && wk[1]->client[1]->posy < monitor->posy ? 
    wk[1]->client[1]->posy : monitor->posy };
   

  cascade(&x, &y, WIDTH, HEIGHT, monitor->sizex, monitor->sizey);
  client_t next = { 
    .w = W, 
    //.shadow = init_window(WIDTH, HEIGHT), 
    .posx = x,
    .posy = y,
    .sizex = WIDTH,
    .sizey = HEIGHT, 
    .gc = init_gc(),
  };

  Window currw = { wk[1]->client[1] ? wk[1]->client[1]->w : -1 };
  // May realloc
  client_t* c = { map_dev(&wk[1]->clients, &next) };
  // Revalidate client
  if (wk[1]->client[1]) {
    client_t* c = { beg(&wk[1]->clients) };
    for (; c != wk[1]->clients.end && c->w != currw; c++);
    wk[1]->client[1] = c;
    unfocus(wk[1]->client[1]->w);
    //unmapwindow(wk[1]->client[1]->shadow);
  }
  
  set_bdrwidth(W, BDR_PX);
  movewindow(W, x, y);
  mapwindow(W);
  //static const unsigned SH = { 10 + BDR_PX };
  //movewindow(c->shadow, x + SH, y + SH);
  //mapwindow(c->shadow);
  focus(c->w);
  wk[1]->client[0] = wk[1]->client[1];
  wk[1]->client[1] = c;
}

static void notify_monitor() {
  char S[4];
  snprintf(S, sizeof S, "%lu", dist(&monitors, monitor) + 1);
  dbus_send("Monitor", S, NORMAL, 1000);
}

void motionnotify(const long X, const long Y, const long TIME) {
  fprintf(stdout, "Motion on root (%ld, %ld)\n", X, Y);
  static Time time;
  if (TIME - time < 1000)
    return;

  time = TIME;
  if (X == monitor->posx || Y == monitor->posy) {
    monitor = prev_dev(&monitors, monitor);
    notify_monitor();
  } else if (X == monitor->posx + monitor->sizex || 
      Y == monitor->posy + monitor->sizey) {
    monitor = next_dev(&monitors, monitor);
    notify_monitor();
  }
  // determine when pointer has moved off size coords of monitor
  if (X > monitor->posx + monitor->sizex)
    warp_pointer(monitor->posx + monitor->sizex, Y);
  if (Y > monitor->posy + monitor->sizey)
    warp_pointer(X, monitor->posy + monitor->sizey);
  // Translation of pointer coords in Xephyr??
}

void keypress(const long MOD, const long KEYSYM, const long) {
  for (size_t i = { 0 }; i < KBDLEN; i++)
    if (KBD[i].mod == MOD && KBD[i].key == KEYSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call]();
      } else if (KBD[i].cmd) {
        fprintf(stdout, "spawn %s\n", KBD[i].cmd);
        spawn(KBD[i].cmd);
      }
    }
}

void btnpress(const long W, const long MOD, const long CODE) {

}

void enternotify(const long, const long, const long) {

}

void propertynotify(const long, const long, const long) {
  refresh_panel();
}

void exposeroot(const long, const long, const long) {
  //for (monitor_t* m = { beg(&monitors) }; m != monitors.end; m++)
    //refresh_rootw(m->posx, m->posy, m->sizex - 1, m->posy + m->sizey - 1);
  
  //refresh_rootw(0, 0, 50, 50);
  //refresh_rootw(500, 0, 50, 50);
  refresh_panel();
}

void switch_wks(const long N, const long, const long) {
  if (wk[1] == itr(&wks, N))
    return;

  for (client_t* c = { beg(&wk[1]->clients) }; c != wk[1]->clients.end; c++) {
    unmapwindow(c->w);
    //unmapwindow(c->shadow);
  }

  setprop_wks(N);
  wk[0] = wk[1];
  wk[1] = itr(&wks, N);
  for (client_t* c = { beg(&wk[1]->clients) }; c != wk[1]->clients.end; c++) {
    //mapwindow(c->shadow);
    mapwindow(c->w);
  }

  refresh_panel();
}

// Utils

void unfocus(const Window W) {
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < BTNLEN; i++) {
    const int MOD = { BTN[i].mod & MODMASK };
    const int KEY = { BTN[i].key };
    ungrab_btn(W, MOD, KEY);
  }
  
  if (wk[1]->client[1]) {
    delprop_actwindow(wk[1]->client[1]->w);
    set_bdrcolor(wk[1]->client[1]->w, INACTBDR);
  }
}

void focus(const Window W) {
  focusin(W);
  set_bdrcolor(W, ACTBDR);
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < BTNLEN; i++) {
    const int MOD = { BTN[i].mod & MODMASK };
    const int KEY = { BTN[i].key };
    ungrab_btn(W, MOD, KEY);
    grab_btn(W, MOD, KEY);
  }
}

void switchwks(const unsigned N) {
  if (N > 0 && N <= NWKS) {
    if (!send_switchwks(N - 1))
      fprintf(stderr, "Failed to send switchwks event\n");
  } else if (N == 0) {

  }
}

void switchmon(const unsigned N) {
  if (N > 0 && N <= monitors.size) {
    monitor = itr(&monitors, N - 1);
    warp_pointer(monitor->posx + 4, monitor->posy + 4);
    char S[4];
    snprintf(S, sizeof S, "%d", N);
    dbus_send("Monitor", S, NORMAL, 1000);
  } else if (N == 0) {

  }
}

static void get_icon(char* S, const size_t L, const Window W) {
  static const size_t ICONLEN = { 4 };
  unsigned char* icon = { NULL };
  /*
  if (getprop_icon(&icon, W, 0, 1)) {
    fprintf(stdout, "Icon width prop %d\n", *icon);
    deinit_prop(icon);
  }
  
  if (getprop_icon(&icon, W, 1, 1)) {
    fprintf(stdout, "Icon height prop %d\n", *icon);
    deinit_prop(icon);
  }
  */
  //create_pixmap((char*) icon + 2, *icon, *(icon + 1), FG1, BG1);
  if (getprop_iconname(&icon, W, 0, ICONLEN)) {
    snprintf(S, ICOLEN < L ? ICOLEN : L, "%s", icon);
    deinit_prop(icon);
  }
}

void refresh_panel() {
  char S[32];
  unsigned x = { 0 };
  const monitor_t* M = { beg(&monitors) };
  for (wks_t* w = { beg(&wks) }; w != wks.end; w++) {
    snprintf(S, sizeof S, "%c", w->clients.size ? '*' : ' ');
    draw_element(S, wksgc, FG1, w == wk[1] ? BG2 : BG1, &x, M->sizex, 
      M->sizey);
    snprintf(S, sizeof S, "");
    draw_element(S, wksgc, 0, FG1, &x, M->sizex, M->sizey);
  }
  
  snprintf(S, sizeof S, "");
  draw_element(S, wksgc, 0, BG1, &x, M->sizex, M->sizey);
  for (client_t* c = { beg(&wk[1]->clients) }; c != wk[1]->clients.end; c++) {
    get_icon(S, sizeof S, c->w); 
    draw_element(S, c->gc, FG1, c == wk[1]->client[1] ? BG2 : BG1, &x, 
      M->sizex, M->sizey);
  }
  
  if (wk[1]->clients.size) {
    snprintf(S, sizeof S, "");
    draw_element(S, statusgc, 0, BG1, &x, M->sizex, M->sizey);
  }

  draw_element(WMNAME, statusgc, FG1, BG1, &x, M->sizex, M->sizey);
}

client_t* prev_client(client_t* client, const bool FL) {
  unfocus(client->w);
  //unmapwindow(client->shadow);
  client_t* c = { FL ? prev_dev(&wk[1]->clients, client) :
    next_dev(&wk[1]->clients, client) };
  //mapwindow(c->shadow);
  focus(c->w);
  return c;
}

// Commands

void wks0() {
  switchwks(0);
}

void wks1() {
  switchwks(1);
}

void wks2() {
  switchwks(2);
}

void wks3() {
  switchwks(3);
}

void wks4() {
  switchwks(4);
}

void wks5() {
  switchwks(5);
}

void wks6() {
  switchwks(6);
}

void wks7() {
  switchwks(7);
}

void wks8() {
  switchwks(8);
}

void wks9() {
  switchwks(9);
}

void mon0() {
  switchmon(0);
}

void mon1() {
  switchmon(1);
}

void mon2() {
  switchmon(2);
}

void mon3() {
  switchmon(3);
}

void mon4() {
  switchmon(4);
}

void mon5() {
  switchmon(5);
}

void mon6() {
  switchmon(6);
}

void mon7() {
  switchmon(7);
}

void mon8() {
  switchmon(8);
}

void mon9() {
  switchmon(9);
}

void kill_curr() {
  fprintf(stdout, "Kill\n");
  if (wk[1]->client[1]) {
    if (!send_killmsg(wk[1]->client[1]->w))
      fprintf(stderr, "Failed to send kill event\n");
  }
}

void mode() {
  // Minimize (unmap)
  // Restore
  // Maximize to current monitor
}

void prev() {
  wk[1]->client[0] = wk[1]->client[1];
  wk[1]->client[1] = wk[1]->client[1] ? prev_client(wk[1]->client[1], true) : 
    NULL;
}

void next() {
  wk[1]->client[0] = wk[1]->client[1];
  wk[1]->client[1] = wk[1]->client[1] ? prev_client(wk[1]->client[1], false) :
    NULL;
}

void quit() {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}
