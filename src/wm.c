#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <Xlib.h>
#include <../config.h>

static const size_t KBDLEN = { sizeof KBD / sizeof KBD[0] };
static const size_t BTNLEN = { sizeof BTN / sizeof BTN[0] };
static blk_t clients;
static blk_t monitors;
static client_t* client[2]; // Prev, Curr
// Init monitors
// Set curr monitor
// Uniform virt vport dpywidth(), dpyheight() to span all monitors
// WKs to span all monitors
// Arrangements, Modes to operate on selection per monitor
static monitor_t* monitor;
static unsigned wks = { 1 };
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
  [KILL] = kill,
  [TOGGLEMODE] = mode,
  [PREVCLI] = prev,
  [NEXTCLI] = next,
  [QUIT] = quit,
};

static GC rootgc;
static GC wksgc;

// Inits

bool init_wm() {
  // Initial reserve is working correctly
  clients = init_blk(sizeof(client_t), 2);
  if (beg(&clients) == NULL) 
    return false;

  // Initial reserve is working correctly
  monitors = init_blk(sizeof(monitor_t), 2);
  if (beg(&monitors) == NULL) {
    deinit_blk(&clients);
    return false;
  }

  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < KBDLEN; i++) {
    const int MOD = { KBD[i].mod & MODMASK };
    const int KEY = { KBD[i].key };
    grab_key(MOD, KEY);
  }

  rootgc = init_gc();
  wksgc = init_gc();
  fprintf(stdout, "Init %s\n", WMNAME);
  return true;
}

void deinit_wm() {
  // Blocks are deinit immediately upon failure so no checks here
  // Deinit client elements
  for (client_t* c = { beg(&clients) }; c != clients.end; c++)
    deinit_gc(c->gc);
  
  deinit_blk(&clients);
  deinit_blk(&monitors);
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < KBDLEN; i++) {
    const int MOD = { KBD[i].mod & MODMASK };
    const int KEY = { KBD[i].key };
    ungrab_key(MOD, KEY);
  }
  
  deinit_gc(wksgc);
  deinit_gc(rootgc);
  fprintf(stdout, "Deinit %s\n", WMNAME);
}

void init_wks() {
  // Workspaces
  set_nwks(NWKS);
  set_wks(0);
}

// Event calls

void noop(const long, const long, const long) {

}

void mapnotify(const long, const long, const long) {

}

void unmapnotify(const long W, const long, const long) {
  // Validate client
  if (!client[1] || client[1]->w != W)
    return;
  
  client[1] = prev_client(client[1], true);
  clear_clientlist();
  client_t* c;
  for (client_t* client = { beg(&clients) }; client != clients.end; client++) {
    if (client->w == W)
      c = client;
    else 
      app_clientlist(client->w);
  }
  // No fail  
  destroy_window(c->shadow);
  deinit_gc(c->gc);
  unmap_dev(&clients, c);
  client[1] = clients.size == 0 ? NULL : client[1];
}
/*
void clientmessage(const long W, const long, const long) {

}
*/

void configureroot(const long, const long WIDTH, const long HEIGHT) {
  fprintf(stdout, "Config root %ld, %ld", WIDTH, HEIGHT);
}

void configurenotify(const long W, const long WIDTH, const long HEIGHT) {

}

void maprequest(const long W, const long WIDTH, const long HEIGHT) {
  int x = { client[1] ? client[1]->posx : 0 };
  int y = { client[1] ? client[1]->posy : 0 };
  cascade(&x, &y, WIDTH, HEIGHT);
  client_t next = { 
    .w = W, 
    .shadow = init_shadow(WIDTH, HEIGHT), 
    .posx = x,
    .posy = y,
    .sizex = WIDTH,
    .sizey = HEIGHT, 
    .gc = init_gc(), 
    .wks = wks
  };

  Window currw = { client[1] ? client[1]->w : -1 };
  // May realloc
  client_t* c = { map_dev(&clients, &next) };
  // Revalidate client
  if (client[1]) {
    client_t* c = { beg(&clients) };
    for (; c != clients.end && c->w != currw; c++);
    client[1] = c;
    unfocus(client[1]->w);
    unmapwindow(client[1]->shadow);
  }
  
  set_bdrwidth(W, BDR_PX);
  movewindow(W, x, y);
  mapwindow(W);
  movewindow(c->shadow, x + 14, y + 14);
  mapwindow(c->shadow);
  focus(c->w);
  client[1] = c;
}

void keypress(const long W, const long STATE, const long CODE) {
  const int KMOD = { STATE & modmask() };
  const int KSYM = { keycode2sym(CODE) };
  for (size_t i = { 0 }; i < KBDLEN; i++)
    if (KBD[i].mod == KMOD && KBD[i].key == KSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call]();
      } else if (KBD[i].cmd) {
        fprintf(stdout, "spawn %s\n", KBD[i].cmd);
        spawn(KBD[i].cmd);
      }
    }
}

void btnpress(const long, const long, const long) {

}

void enternotify(const long, const long, const long) {

}

void propertynotify(const long, const long, const long) {
  refresh_panel();
}

void switch_wks(const long N, const long, const long) {
  set_wks(N);
  wks = N + 1;
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
  
  if (client[1]) {
    del_actwindow(client[1]->w);
    set_bdrcolor(client[1]->w, INACTBDR);
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

void refresh_panel() {
  char S[32];
  {
    unsigned offset = { 0 };
    snprintf(S, sizeof S, "%d/%d", wks, NWKS);
    draw_wks(S, wksgc, FG1, BG2, &offset);
    draw_root(WMNAME, rootgc, FG1, BG1, &offset);
  }

  if (clients.size == 0)
    return;
  {
    unsigned offset = { 0 };
    for (client_t* c = { beg(&clients) }; c != clients.end; c++) {
      snprintf(S, sizeof S, "W %lu", c->w);
      draw_client(S, c->gc, FG1, c == client[1] ? BG2 : BG1, &offset);
    }
  }
}

client_t* prev_client(client_t* client, const bool FL) {
  unfocus(client->w);
  unmapwindow(client->shadow);
  client_t* c = { FL ? prev_dev(&clients, client) :
    next_dev(&clients, client) };
  mapwindow(c->shadow);
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

void kill() {
  fprintf(stdout, "Kill\n");
  if (client[1]) {
    if (!send_killmsg(client[1]->w))
      fprintf(stderr, "Failed to send kill event\n");
  }
}

void mode() {
  // Minimize (unmap)
  // Restore
  // Maximize to current monitor
}

void prev() {
  client[1] = client[1] ? prev_client(client[1], true) : NULL;
}

void next() {
  client[1] = client[1] ? prev_client(client[1], false) : NULL;
}

void quit() {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}
