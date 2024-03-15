#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <Xlib.h>
#include <util.h>
#include <../config.h>

typedef struct {
  Window w;
  Window shadow;
  pair_t pos;
  pair_t size;
  GC gc;
  unsigned wks;
  int sel;
  int ft;
  int pad[3];
} client_t;

typedef struct {
  blk_t CLIENTS[NWKS];
  pair_t size;
} monitor_t;

static blk_t clients;
static blk_t monitors;
static client_t* client;
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
  [PREVCLI] = prev,
  [NEXTCLI] = next,
  [QUIT] = quit,
};

static GC rootgc;
static GC wksgc;
static pair_t dpysize;
static unsigned wkssize;
static unsigned clientssize;
static unsigned rootsize;
static unsigned panelheight;
static unsigned hpad;
static unsigned vpad;

// Inits

bool init_wm() {
  // Initial reserve is working correctly
  clients = init_blk(sizeof(client_t), 2);
  if (clients.blk == NULL) 
    return false;

  // Initial reserve is working correctly
  monitors = init_blk(sizeof(monitor_t), 2);
  if (monitors.blk == NULL) {
    deinit_blk(&clients);
    return false;
  }

  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < LEN(KBD); i++) {
    const int MOD = { KBD[i].mod & MODMASK };
    const int KEY = { KBD[i].key };
    grab_key(MOD, KEY);
  }

  rootgc = init_gc();
  wksgc = init_gc();
  dpysize = (pair_t) { dpywidth(), dpyheight() };
  wkssize = WKS_PERC * dpysize.x;
  clientssize = CLIENTS_PERC * dpysize.x;
  rootsize = ROOT_PERC * dpysize.x;
  panelheight = VPANEL_PERC * dpysize.y;
  hpad = HPAD_PERC * dpysize.x;
  vpad = VPAD_PERC * dpysize.y;
  fprintf(stdout, "Init %s\n", WMNAME);
  return true;
}

void deinit_wm() {
  // Blocks are deinit immediately upon failure so no checks here
  // Deinit client elements
  for (client_t* client = { clients.blk }; 
    client < (client_t*) clients.blk + clients.size; client++)
    deinit_gc(client->gc);
  
  deinit_blk(&clients);
  deinit_blk(&monitors);
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < LEN(KBD); i++) {
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
  if (!client || client->w != W)
    return;

  fprintf(stdout, "unmap(): window %ld\n", W);
  fprintf(stdout, "unmap(): window %ld\n", client->w);
  destroy_window(client->shadow);
  deinit_gc(client->gc);
  unmap_dev(&clients, client);
  prev();
  clear_clientlist();
  for (client_t* client = { clients.blk };
    client < (client_t*) clients.blk + clients.size; client++)
    app_clientlist(client->w);
}
/*
void clientmessage(const long W, const long, const long) {

}
*/

void configurenotify(const long, const long, const long) {

}

void maprequest(const long W, const long WIDTH, const long HEIGHT) {
  const pair_t POS = { clients.size ? client->pos.x + panelheight : 0, 
    clients.size ? client->pos.y + panelheight : panelheight };
  const pair_t SIZE = { WIDTH, HEIGHT };
  client_t client_ = { 
    .w = W, 
    .shadow = init_shadow(WIDTH, HEIGHT), 
    .pos = POS, 
    .size = SIZE, 
    .gc = init_gc(), 
    .wks = wks
  };

  Window curr = { client ? client->w : -1 };
  // May realloc
  client_t* client_p = { map_dev(&clients, &client_) };
  // Revalidate client
  if (client) {
    client_t* client_ = { clients.blk };
    for (;
      client_ < (client_t*) clients.blk + clients.size && client_->w != curr; 
      client_++);
    client = client_;
  }

  if (client_p) {
    if (client) {
      unfocus(client->w);
      unmapwindow(client->shadow);
    }

    focus(client_p->w);
    client = client_p;
    set_bdrwidth(W, BDR_PX);
    movewindow(W, POS.x, POS.y);
    movewindow(client->shadow, POS.x + 14, POS.y + 14);
    mapwindow(client->shadow);
    mapwindow(W);
  }
}

void keypress(const long W, const long STATE, const long CODE) {
  const int KMOD = { STATE & modmask() };
  const int KSYM = { keycode2sym(CODE) };
  for (size_t i = { 0 }; i < LEN(KBD); i++)
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

void msg1(const long W, const long, const long) {

}

void msg2(const long W, const long, const long) {

}

// Utils

void unfocus(const Window W) {
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < LEN(BTN); i++) {
    const int MOD = { BTN[i].mod & MODMASK };
    const int KEY = { BTN[i].key };
    ungrab_btn(W, MOD, KEY);
  }
  
  if (client) {
    del_actwindow(client->w);
    set_bdrcolor(client->w, INACTBDR);
  }
}

void focus(const Window W) {
  focusin(W);
  set_bdrcolor(W, ACTBDR);
  const int MODMASK = { modmask() };
  for (size_t i = { 0 }; i < LEN(BTN); i++) {
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
  const unsigned DPYW = { dpysize.x };
  const unsigned DPYH = { dpysize.y };
  char S[32];
  snprintf(S, sizeof S, "%d/%d", wks, NWKS);
  draw_element(wksgc, FG1, BG1, 0, DPYH - panelheight, wkssize + hpad, 
    panelheight);
  print_element(wksgc, S, 0, hpad, DPYH, vpad);
  draw_element(rootgc, FG2, BG1, wkssize, DPYH - panelheight, dpysize.x, 
    panelheight);
  print_element(rootgc, WMNAME, wkssize, hpad, DPYH, vpad);

  if (clients.size == 0)
    return;
  else if (clientssize / clients.size < 10) {
    if (client) {
      snprintf(S, sizeof S, "W %lu", client->w);
      draw_element(client->gc, FG1, BG0, 0, 0, clientssize, panelheight);
      print_element(client->gc, S, 0, hpad, panelheight, vpad);
    }
  } else {
    const unsigned WSIZE = { DPYW / clients.size };
    for (client_t* client_ = { clients.blk }; 
        client_ < (client_t*) clients.blk + clients.size; client_++) {
      const size_t D = { client_ - (client_t*) clients.blk };
      snprintf(S, sizeof S, "W %lu", client_->w);
      draw_element(client_->gc, FG1, client_ == client ? BG2 : BG1, 
        D * WSIZE, 0, WSIZE, panelheight);
      print_element(client_->gc, S, D * WSIZE, hpad, panelheight, vpad);
    }
  }
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
  if (client) {
    if (!send_killmsg(client->w))
      fprintf(stderr, "Failed to send kill event\n");
  }
}

void prev() {
  fprintf(stdout, "Prev\n");
  if (!clients.size)
    return;

  client_t* client_p = { prev_dev(&clients, client) };
  unfocus(client->w);
  unmapwindow(client->shadow);
  mapwindow(client_p->shadow);
  focus(client_p->w);
  client = client_p;
}

void next() {
  fprintf(stdout, "Next\n");
  if (!clients.size)
    return;

  client_t* client_p = { next_dev(&clients, client) };
  unfocus(client->w);
  unmapwindow(client->shadow);
  mapwindow(client_p->shadow);
  focus(client_p->w);
  client = client_p;
}

void quit() {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}
