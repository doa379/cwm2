#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <Xlib.h>
#include <draw.h>
#include <../config.h>

typedef struct client_s {
  Window w;
  Window parw;
  Window shadow;
  GC gc;
  unsigned posx;
  unsigned posy;
  unsigned sizex;
  unsigned sizey;
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

static Window rootw;
static Window panel;
static GC wksgc;
static GC statusgc;

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

void init_wm(const Window W) {
  rootw = W;
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
  {
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
  }

  wksgc = init_gc();
  statusgc = init_gc();
  fprintf(stdout, "Init %s\n", WMNAME);
}

void deinit_wm() {
  deinit_gc(statusgc);
  deinit_gc(wksgc);
  // Restore X11 states to defaults
  for (wks_t* w = { beg(&wks) }; w != wks.end; w++)
    for (client_t* c = { beg(&w->clients) }; c != w->clients.end; c++) {
      //set_bdrwidth(c->w, 0);
      mapwindow(c->parw);
      mapwindow(c->w);
      reparent(c->w, rootw);
      deinit_window(c->parw);
      //deinit_window(c->shadow);
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
  
  reparent(W, rootw);
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
  deinit_gc(c->gc);
  deinit_window(c->parw);
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
  // Set origin
  int x = { wk[1]->client[1] && wk[1]->client[1]->posx < monitor->posx ? 
    wk[1]->client[1]->posx : monitor->posx };
  int y = { wk[1]->client[1] && wk[1]->client[1]->posy < monitor->posy ? 
    wk[1]->client[1]->posy : monitor->posy };
   
  cascade(&x, &y, WIDTH, HEIGHT, monitor->sizex, monitor->sizey);

  client_t next = { 
    .w = W, 
    .parw = par(W, x, y, WIDTH, HEIGHT + vh(), BDR_PX, 0x0, BG1),
    //.shadow = init_window(WIDTH, HEIGHT),
    .gc = init_gc(),
    .posx = x,
    .posy = y,
    .sizex = WIDTH,
    .sizey = HEIGHT, 
  };

  Window currw = { wk[1]->client[1] ? wk[1]->client[1]->w : -1 };
  // May realloc
  client_t* c = { map_dev(&wk[1]->clients, &next) };
  // Revalidate client
  if (wk[1]->client[1]) {
    client_t* c = { beg(&wk[1]->clients) };
    for (; c != wk[1]->clients.end && c->w != currw; c++);
    wk[1]->client[1] = c;
    unfocus(wk[1]->client[1]->parw);
    //unmapwindow(wk[1]->client[1]->shadow);
  }
  
  set_bdrwidth(W, 0);
  //movewindow(W, x, y);
  mapwindow(c->parw);
  mapwindow(W);
  
  //static const unsigned SH = { 10 + BDR_PX };
  //movewindow(c->shadow, x + SH, y + SH);
  //mapwindow(c->shadow);
  focus(c->parw);
  wk[1]->client[0] = wk[1]->client[1];
  wk[1]->client[1] = c;
}

void motionnotify(const long X, const long Y, const long TIME) {
  //fprintf(stdout, "Motion on root (%ld, %ld)\n", X, Y);
  static Time time;
  if (TIME - time < 1000)
    return;

  time = TIME;
  if (X == monitor->posx || Y == monitor->posy) {
    monitor = prev_dev(&monitors, monitor);
    fprintf(stdout, "Monitor %lu\n", dist(&monitors, monitor) + 1);
  } else if (X == monitor->posx + monitor->sizex || 
      Y == monitor->posy + monitor->sizey) {
    monitor = next_dev(&monitors, monitor);
    fprintf(stdout, "Monitor %lu\n", dist(&monitors, monitor) + 1);
  }
  // determine when pointer has moved off size coords of monitor
  if (X > monitor->posx + monitor->sizex)
    warp_pointer(monitor->posx + monitor->sizex, Y);
  if (Y > monitor->posy + monitor->sizey)
    warp_pointer(X, monitor->posy + monitor->sizey);
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
  char S[32];
  for (client_t* c = { beg(&wk[1]->clients) }; c != wk[1]->clients.end; c++) {
    get_name(S, sizeof S, c->w);
    draw_string(S, c->parw, c->gc, FG1, 4, vh());
  }
}

void exposeroot(const long, const long, const long) {
  refresh_panel();
}

void switch_wks(const long N, const long, const long) {
  if (wk[1] == itr(&wks, N))
    return;

  for (client_t* c = { beg(&wk[1]->clients) }; c != wk[1]->clients.end; c++) {
    unmapwindow(c->w);
    unmapwindow(c->parw);
    //unmapwindow(c->shadow);
  }

  setprop_wks(N);
  wk[0] = wk[1];
  wk[1] = itr(&wks, N);
  for (client_t* c = { beg(&wk[1]->clients) }; c != wk[1]->clients.end; c++) {
    //mapwindow(c->shadow);
    mapwindow(c->parw);
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
    delprop_active(wk[1]->client[1]->w);
    set_bdrcolor(wk[1]->client[1]->w, INACTBDR);
  }
}

void focus(const Window W) {
  focusin(W);
  setprop_state(W);
  setprop_active(W);
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

void switchmon(const size_t N) {
  if (N > 0 && N <= monitors.size) {
    monitor = itr(&monitors, N - 1);
    warp_pointer(monitor->posx + 4, monitor->posy + 4);
    fprintf(stdout, "Monitor %lu\n", N);
  } else if (N == 0) {

  }
}

void get_name(char* S, const size_t L, const Window W) {
  static const size_t LEN = { 32 };
  unsigned char* name = { NULL };
  if (getprop_utfname(&name, W, 0, LEN) || getprop_name(&name, W, 0, LEN)) {
    snprintf(S, LEN < L ? LEN : L, "%s", name);
    deinit_prop(name);
  }
}

void get_icon(char* S, const size_t L, const Window W) {
  static const size_t LEN = { 4 };
  unsigned char* icon = { NULL };
  if (getprop_iconname(&icon, W, 0, LEN)) {
    snprintf(S, LEN < L ? LEN : L, "%s", icon);
    deinit_prop(icon);
  }
}

void refresh_panel() {
  static const unsigned BDRW = { 1 };
  unsigned x = { 0 };
  char S[32];
  const monitor_t* M = { beg(&monitors) };
  fill_element(wksgc, Gray70, 0, M->sizey - vh(), M->sizey, vh());
  for (wks_t* w = { beg(&wks) }; w != wks.end; w++) {
    
    if (w->clients.size) {
      if (w == wk[1]) {
        
        unsigned slen = { 0 };
        for (client_t* c = { beg(&w->clients) }; c != w->clients.end; c++) {
          get_icon(S, sizeof S, c->w);
          slen += string_len(S);
        }

        fill_element(wksgc, FG1, x, M->sizey - vh(), slen, vh());
        unsigned x0 = { x };
        for (client_t* c = { beg(&w->clients) }; c != w->clients.end; c++) {
          get_icon(S, sizeof S, c->w);
          const unsigned SLEN = { string_len(S) };
          fill_element(wksgc, c == w->client[1] ? BG2 : BG1,
            x0 + 1, M->sizey - vh() + BDRW, 
            SLEN, vh() - 2 * BDRW);
          draw_string(S, rootw, wksgc, FG1, x0 + 4, M->sizey);
          x0 += SLEN;
        }

        x += x0 - 1;
        
      } else {
        
        char ICON[32];
        get_icon(ICON, sizeof ICON, w->client[1]->w);
        snprintf(S, sizeof S, "(%lu) %s", w->clients.size, ICON);
        const unsigned SLEN = { string_len(S) };
        fill_element(wksgc, FG1, x, M->sizey - vh(), SLEN, vh());
        fill_element(wksgc, BG1,
            x + BDRW, M->sizey - vh() + BDRW, 
            SLEN - 2 * BDRW, vh() - 2 * BDRW);
        draw_string(S, rootw, wksgc, FG1, x + 4, M->sizey);
        x += SLEN - 1;
        
      }

    } else {
      S[0] = ' ';
      S[1] = '\0';
      const unsigned SLEN = { string_len(S) + 4 };
      fill_element(wksgc, FG1, x, M->sizey - vh(), SLEN, vh());
      fill_element(wksgc, w == wk[1] ? BG2 : BG1,
        x + BDRW, M->sizey - vh() + BDRW, 
        SLEN - 2 * BDRW, vh() - 2 * BDRW);
      draw_string(S, rootw, wksgc, FG1, x + 4, M->sizey);
      x += SLEN - 1;
    }
  }

  const unsigned SLEN = { string_len(WMNAME) };
  fill_element(statusgc, Gray70, 
    M->sizex - SLEN, M->sizey - vh(),
    SLEN, vh());
  draw_string(WMNAME, rootw, statusgc, FG1, M->sizex - SLEN, M->sizey);


 /* 
  if (M->sizex - SLEN - x < 8) {
    if (wk[1]->clients.size) {
      snprintf(S, sizeof S, "%lu/%lu",
        dist(&wk[1]->clients, wk[1]->client[1]) + 1, wk[1]->clients.size);
      draw_string(S, clientsgc, FG1, x + 4, M->sizey);
    }
  } else {
    for (client_t* c = { beg(&wk[1]->clients) }; 
        c != wk[1]->clients.end; c++) {
      get_icon(S, sizeof S, c->w);
      fprintf(stdout, "%s\n", S);
      const unsigned SLEN = { string_len(S) };
      draw_string(S, clientsgc, FG1, x, M->sizey);
      x += SLEN + 1;
    }
  }
  */
}

client_t* prev_client(client_t* client, const bool FL) {
  unfocus(client->parw);
  //unmapwindow(client->shadow);
  client_t* c = { FL ? prev_dev(&wk[1]->clients, client) :
    next_dev(&wk[1]->clients, client) };
  //mapwindow(c->shadow);
  focus(c->parw);
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
    reparent(wk[1]->client[1]->w, rootw);
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
