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
  int sel;
  int ft;
  int pad[6];
} client_t;

typedef struct {
  blk_t CLIENTS[NWKS];
  pair_t size;
} monitor_t;

static blk_t clients;
static blk_t monitors;
static client_t* client;
static monitor_t* monitor;
static unsigned wks;
static void (*CALLFN[])() = {
  [QUIT] = quit,
  [KILL] = kill,
  [PREVCLI] = prev,
  [NEXTCLI] = next
};

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
}

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
  client_t client_ = { W, POS, SIZE, init_gc() };
  client_t* client_p = map_dev(&clients, &client_);
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

}

static void unfocus(const Window W) {
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

static void focus(const Window W) {
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

////////////////////////////////////////////////////////////////////////////////
// Commands                                                                   //
////////////////////////////////////////////////////////////////////////////////
static void quit() {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill() {
  fprintf(stdout, "Kill\n");
  if (client)
    send_killmsg(client->w);
}

static void prev() {
  fprintf(stdout, "Prev\n");
  client_t* client_p = prev_dev(&clients, client);
  unfocus(client->w);
  focus(client_p->w);
  client = client_p;
}

static void next() {
  fprintf(stdout, "Next\n");
  client_t* client_p = next_dev(&clients, client);
  unfocus(client->w);
  focus(client_p->w);
  client = client_p;
}
