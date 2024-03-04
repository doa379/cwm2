#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <Xlib.h>
#include <util.h>
//#include <X11/XKBlib.h>
#include <../config.h>

typedef struct {
  Window w;
  pair_t pos;
  pair_t size;
  GC gc;
  // 32B
  int sel;
  int ft;
  int m0;
  int m1;
  int m2;
  int m3;
  int m4;
  int m5;
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

  //XUngrabKey(dpy, AnyKey, AnyModifier, W);
  const int MODMASK = modmask();
  for (size_t i = 0; i < LEN(KBD); i++) {
    const int MOD = KBD[i].mod;
    const int KEY = KBD[i].key;
    //XGrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, W, 
      //true, GrabModeAsync, GrabModeAsync);
    grab_key(MOD & MODMASK, KEY);
  }

  //atom = init_atoms(dpy);
  init_atoms();
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
    const int MOD = KBD[i].mod;
    const int KEY = KBD[i].key;
    ungrab_key(MOD & MODMASK, KEY);
    //XUngrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, W);
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
  //prev(dpy, ROOTW);

  //XDeleteProperty(dpy, ROOTW, atom.CLIENT_LIST);
  delfrom_clientlist();
  for (client_t* client = clients.blk;
    client < (client_t*) clients.blk + clients.size; client++)
    appto_clientlist(client->w);
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
        if (fork() == 0) {
          //close(ConnectionNumber(dpy));
          setsid();
          system(KBD[i].cmd);
        }
      }
    }

}

void btnpress(const long, const long, const long) {

}

void enternotify(const long, const long, const long) {

}

void propertynotify(const long, const long, const long) {

}

/*
void configure_root(Display* dpy, const Window ROOTW, const int WIDTH, const int HEIGHT) {

}

void map(Display* dpy, const Window ROOTW, const Window W) {
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return;

  const pair_t POS = { clients.size ? client->pos.x + BARH : 0, 
    clients.size ? client->pos.y + BARH : BARH };
  const pair_t SIZE = { wa.width, wa.height };
  client_t client_ = { W, POS, SIZE, init_gc(dpy, ROOTW) };
  client_t* client_p = map_dev(&clients, &client_);
  if (client_p) {
    if (client)
      unfocus(dpy, ROOTW, client->w);

    focus(dpy, ROOTW, client_p->w);
    client = client_p;

    static const int WMASK = EnterWindowMask | 
      FocusChangeMask |
      PropertyChangeMask | 
      StructureNotifyMask;
    XSelectInput(dpy, W, WMASK);
    XChangeProperty(dpy, ROOTW, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
      (unsigned char*) &W, 1);
    XMoveWindow(dpy, W, POS.x, POS.y);
    XMapWindow(dpy, W);
  }
}

void unmap(Display* dpy, const Window ROOTW, const Window W) {
  // Validate client
  if (!client || client->w != W)
    return;

  fprintf(stdout, "unmap(): window %ld\n", W);
  fprintf(stdout, "unmap(): window %ld\n", client->w);
  deinit_gc(dpy, client->gc);
  unmap_dev(&clients, client);
  prev(dpy, ROOTW);

  XDeleteProperty(dpy, ROOTW, atom.CLIENT_LIST);
  for (client_t* client = clients.blk; client < clients.blk + clients.size; 
    client++)
    XChangeProperty(dpy, ROOTW, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
      (const unsigned char*) client->w, 1);
}

void key(Display* dpy, const Window W, const int STATE, const int CODE) {
  const int KMOD = STATE & modmask(dpy);
  const int KSYM = XkbKeycodeToKeysym(dpy, CODE, 0, 0);
  for (size_t i = 0; i < LEN(KBD); i++)
    if (KBD[i].mod == KMOD && KBD[i].key == KSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call](dpy, W);
      } else if (KBD[i].cmd) {
        fprintf(stdout, "spawn %s\n", KBD[i].cmd);
        if (fork() == 0) {
          close(ConnectionNumber(dpy));
          setsid();
          system(KBD[i].cmd);
        }
      }
    }
}

static void unfocus(Display* dpy, const Window ROOTW, const Window W) {
  const int MODMASK = modmask(dpy);
  // Deactivates
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, W);
  }
  
  if (client) {
    XDeleteProperty(dpy, client->w, atom.ACTIVE_WINDOW);
    XSetWindowBorder(dpy, client->w, (size_t) INACTBDR);
  }
}

static void focus(Display* dpy, const Window ROOTW, const Window W) {
  // Activates
  XSetWindowBorder(dpy, W, (size_t) ACTBDR);
  XSetWindowBorderWidth(dpy, W, BDRW);
  XSetInputFocus(dpy, W, RevertToPointerRoot, CurrentTime);
  XChangeProperty(dpy, ROOTW, atom.WM_STATE, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XChangeProperty(dpy, W, atom.ACTIVE_WINDOW, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XRaiseWindow(dpy, W);
  static const int BUTTONMASK = ButtonPressMask | ButtonReleaseMask;
  const int MODMASK = modmask(dpy);
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, W);
    XGrabButton(dpy, KEY, MOD & MODMASK, W, false, BUTTONMASK, GrabModeSync, 
      GrabModeSync, None, None);
  }
}
*/

static void unfocus(const Window W) {
  const int MODMASK = modmask();
  // Deactivates
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    //XUngrabButton(dpy, KEY, MOD & MODMASK, W);
    ungrab_btn(W, MOD & MODMASK, KEY);
  }
  
  if (client) {
    //XDeleteProperty(dpy, client->w, atom.ACTIVE_WINDOW);
    del_actwindow(client->w);
    //XSetWindowBorder(dpy, client->w, (size_t) INACTBDR);
    set_bdrcolor(client->w, INACTBDR);
  }
}

static void focus(const Window W) {
  // Activates
  focusin(W);
  set_bdrcolor(W, ACTBDR);


  const int MODMASK = modmask();
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    //XUngrabButton(dpy, KEY, MOD & MODMASK, W);
    ungrab_btn(W, MOD & MODMASK, KEY);
    //XGrabButton(dpy, KEY, MOD & MODMASK, W, false, BUTTONMASK, GrabModeSync, 
      //GrabModeSync, None, None);
    grab_btn(W, MOD & MODMASK, KEY);
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
