#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <Xlib.h>
#include <util.h>
#include <../config.h>

typedef struct {
  Window w;
  pair_t pos;
  pair_t size;
  GC gc;
  // 32B
  unsigned wks;
  int sel;
  int ft;
  int pad[5];
} client_t;

typedef struct {
  blk_t CLIENTS[NWKS];
  pair_t size;
} monitor_t;

static blk_t clients;
static blk_t monitors;
static client_t* client;
static monitor_t* monitor;
static unsigned wks = 1;
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

  const int MODMASK = modmask();
  for (size_t i = 0; i < LEN(KBD); i++) {
    const int MOD = KBD[i].mod & MODMASK;
    const int KEY = KBD[i].key;
    grab_key(MOD, KEY);
  }

  fprintf(stdout, "Init %s\n", WMNAME);
  return true;
}

void deinit_wm() {
  // Blocks are deinit immediately upon failure so no checks here
  // Deinit client elements
  for (size_t n = 0; n < clients.size; n++) {
    client_t* client = (client_t*) clients.blk + n;
    deinit_gc(client->gc);
  }
  
  deinit_blk(&clients);
  deinit_blk(&monitors);
  const int MODMASK = modmask();
  for (size_t i = 0; i < LEN(KBD); i++) {
    const int MOD = KBD[i].mod & MODMASK;
    const int KEY = KBD[i].key;
    ungrab_key(MOD, KEY);
  }
  
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
  deinit_gc(client->gc);
  unmap_dev(&clients, client);
  prev();
  clear_clientlist();
  for (client_t* client = clients.blk;
    client < (client_t*) clients.blk + clients.size; client++)
    app_clientlist(client->w);
}

void clientmessage(const long, const long, const long) {

}

void configurenotify(const long, const long, const long) {

}

void maprequest(const long W, const long WIDTH, const long HEIGHT) {
  const pair_t POS = { clients.size ? client->pos.x + BARH : 0, 
    clients.size ? client->pos.y + BARH : BARH };
  const pair_t SIZE = { WIDTH, HEIGHT };
  client_t client_ = { W, POS, SIZE, init_gc(), wks };
  Window curr = client ? client->w : -1;
  // May realloc
  client_t* client_p = map_dev(&clients, &client_);
  // Revalidate client
  if (client) {
    client_t* client_ = clients.blk;
    for (;
      client_ < (client_t*) clients.blk + clients.size && client_->w != curr; 
      client_++);
    client = client_;
  }

  if (client_p) {
    if (client)
      unfocus(client->w);

    focus(client_p->w);
    client = client_p;
    set_bdrwidth(W, BDRW);
    movewindow(W, POS.x, POS.y);
    mapwindow(W);
  }
}

void keypress(const long W, const long STATE, const long CODE) {
  const int KMOD = STATE & modmask();
  const int KSYM = keycode2sym(CODE);
  for (size_t i = 0; i < LEN(KBD); i++)
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

// Utils

void unfocus(const Window W) {
  const int MODMASK = modmask();
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod & MODMASK;
    const int KEY = BTN[i].key;
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
  const int MODMASK = modmask();
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod & MODMASK;
    const int KEY = BTN[i].key;
    ungrab_btn(W, MOD, KEY);
    grab_btn(W, MOD, KEY);
  }
}

void switchwks(const unsigned N) {
  if (N > 0 && N <= NWKS) {
    send_switchwks(N);
    set_wks(N - 1);
    wks = N;
    refresh_panel();
  } else if (N == 0) {

  }
}

void refresh_panel() {
  char S[32];
  snprintf(S, sizeof S, "%d/%d", wks, NWKS);
  draw_wks(S, BARH, FG1, BG1);
  if (clients.size > 10 && client) {
    snprintf(S, sizeof S, "W %lu", client->w);
    draw_client(client->gc, S, 0, 1, BARH, FG1, BG0);
  } else {
    for (client_t* client_ = clients.blk; 
        client_ < (client_t*) clients.blk + clients.size; client_++) {
      snprintf(S, sizeof S, "W %lu", client_->w);
      draw_client(client_->gc, S, client_ - (client_t*) clients.blk, 
        clients.size, BARH, FG1, client_ == client ? BG0: BG2);
    }
  }

  draw_root(WMNAME, BARH, FG2, BG1);
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
  if (client)
    send_killmsg(client->w);
}

void prev() {
  fprintf(stdout, "Prev\n");
  client_t* client_p = prev_dev(&clients, client);
  unfocus(client->w);
  focus(client_p->w);
  client = client_p;
}

void next() {
  fprintf(stdout, "Next\n");
  client_t* client_p = next_dev(&clients, client);
  unfocus(client->w);
  focus(client_p->w);
  client = client_p;
}

void quit() {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}
