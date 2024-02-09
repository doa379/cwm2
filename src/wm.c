#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <util.h>
#include <X11/XKBlib.h>
#include <../config.h>

typedef struct {
  Atom PROTO;
  Atom NAME;
  Atom DELETE_WINDOW;
  Atom STATE;
  Atom TAKE_FOCUS;
  Atom SUPPORTED;
  Atom WM_STATE;
  Atom WM_NAME;
  Atom ACTIVE_WINDOW;
  Atom WM_STATE_FULLSCREEN;
  Atom WM_WINDOW_TYPE;
  Atom WM_WINDOW_TYPE_DIALOG;
  Atom CLIENT_LIST;
  Atom NUMBER_OF_DESKTOPS;
  Atom WM_DESKTOP;
  Atom CURRENT_DESKTOP;
  Atom SHOWING_DESKTOP;
} atom_t;

typedef struct {
  Window w;
  pair_t pos;
  pair_t size;
  GC gc;
  int sel;
  int ft;
  int misc;
} client_t;

typedef struct {
  void* clients;
  size_t reserve;
  size_t size;
} C_t;

static atom_t atom;
static C_t C;
static void (*CALLFN[128])(Display*, const Window) = {
  [QUIT] = quit,
  [KILL] = kill
};

static bool init_client(const client_t* client) {
  if (C.size < C.reserve) {
    memcpy((client_t*) C.clients + C.size, client, sizeof(client_t));
    C.size++;
    return true;
  } else {
    void* clients = realloc(C.clients, (C.size + C.reserve) * sizeof(client_t));
    if (clients) {
      C.clients = clients;
      memcpy((client_t*) C.clients + C.size, client, sizeof(client_t));
      C.reserve += C.size;
      C.size++;
      return true;
    }
  }

  return false;
}

static void deinit_client(client_t* client) {
  // Patt: move post segment
  int n = 0;
  for (; n < C.size && ((client_t*) C.clients + n)->w != client->w ; n++);
  memcpy((client_t*) C.clients + n, (client_t*) C.clients + (n + 1),
    (C.size - n - 1) * sizeof(client_t));
  C.size--;
}

bool init_clients() {
  static const int MAXNCLI = 4;
  if ((C.clients = malloc(MAXNCLI * sizeof(client_t)))) {
    C.reserve = MAXNCLI;
    C.size = 0;
    return true;
  }

  return false;
}

void deinit_clients() {
  free(C.clients);
  C.reserve = 0;
  C.size = 0;
}

void init_atoms(Display* dpy) {
  atom_t atom_ = {
    XInternAtom(dpy, "WM_PROTOCOLS", false),
    XInternAtom(dpy, "WM_NAME", false),
    XInternAtom(dpy, "WM_DELETE_WINDOW", false),
    XInternAtom(dpy, "WM_STATE", false),
    XInternAtom(dpy, "WM_TAKE_FOCUS", false),
    XInternAtom(dpy, "_NET_SUPPORTED", false),
    XInternAtom(dpy, "_NET_WM_STATE", false),
    XInternAtom(dpy, "_NET_WM_NAME", false),
    XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false),
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false),
    XInternAtom(dpy, "_NET_CLIENT_LIST", false),
    XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false),
    XInternAtom(dpy, "_NET_WM_DESKTOP", false),
    XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false),
    XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false)
  };

  atom = atom_;
}

void init_windows(Display* dpy, const Window root) {
  unsigned n;
  Window rootw, parw;
  Window* w;
  if (XQueryTree(dpy, root, &rootw, &parw, &w, &n)) {
    for (int i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && 
          wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = root;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, root, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(dpy, false);
}

void key(Display* dpy, const Window root, const int STATE, const int CODE) {
  const int KMOD = STATE & modmask(dpy);
  const int KSYM = XkbKeycodeToKeysym(dpy, CODE, 0, 0);
  for (int i = 0; i < LEN(KBD); i++)
    if (KBD[i].mod == KMOD && KBD[i].key == KSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call](dpy, root);
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

void map(Display* dpy, const Window root, const Window W) {
  const pair_t SIZE = init_window(dpy, W);
  if (SIZE.x && SIZE.y) {
    append_window(dpy, root, W, atom.CLIENT_LIST);
    client_t* prev = (client_t*) C.clients + C.size;
    const pair_t POS = { prev ? prev->pos.x + BARH : 0, 
      prev ? prev->pos.y + BARH : BARH };
    XMoveWindow(dpy, W, POS.x, POS.y);
    XMapWindow(dpy, W);
    client_t client = { W, POS, SIZE };
    init_client(&client);
    // set focus
  }
}

static void quit(Display* dpy, const Window root) {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill(Display* dpy, const Window root) {
  fprintf(stdout, "Kill\n");
}
