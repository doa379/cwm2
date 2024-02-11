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
  client_t* prev;
  client_t* curr;
} C_t;

static atom_t atom;
static C_t C;
static void (*CALLFN[])(Display*, const Window) = {
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

static bool init_clients() {
  static const int MAXNCLI = 4;
  if ((C.clients = malloc(MAXNCLI * sizeof(client_t)))) {
    C.reserve = MAXNCLI;
    C.size = 0;
    C.prev = C.curr = C.clients;
    return true;
  }

  return false;
}

static void deinit_clients() {
  free(C.clients);
  C.reserve = 0;
  C.size = 0;
  C.prev = C.curr = NULL;
}

bool init_wm(Display* dpy, const Window ROOT) {
  atom = init_atoms(dpy);
  init_windows(dpy, ROOT);
  return init_clients();
}

void deinit_wm() {
  deinit_clients();
}

void key(Display* dpy, const Window ROOT, const int STATE, const int CODE) {
  const int KMOD = STATE & modmask(dpy);
  const int KSYM = XkbKeycodeToKeysym(dpy, CODE, 0, 0);
  for (int i = 0; i < LEN(KBD); i++)
    if (KBD[i].mod == KMOD && KBD[i].key == KSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call](dpy, ROOT);
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

void map(Display* dpy, const Window ROOT, const Window W) {
  const pair_t SIZE = init_window(dpy, W);
  if (SIZE.x && SIZE.y) {
    append_window(dpy, ROOT, W, atom.CLIENT_LIST);
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

static void init_windows(Display* dpy, const Window ROOT) {
  Window root;
  Window par;
  Window* w;
  unsigned n;
  if (XQueryTree(dpy, ROOT, &root, &par, &w, &n)) {
    for (int i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && 
          wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = ROOT;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, ROOT, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(dpy, false);
}

static void quit(Display* dpy, const Window ROOT) {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill(Display* dpy, const Window ROOT) {
  fprintf(stdout, "Kill\n");
}
