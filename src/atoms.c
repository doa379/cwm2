#include <X11/Xatom.h>
#include <atoms.h>

static Display* dpy;
static Window rootw;
static Atom ATOM[NPROPS];

void init_atoms(Display* dpy_) {
  dpy = dpy_;
  rootw = XRootWindow(dpy, DefaultScreen(dpy));
  ATOM[WM_PROTOCOLS] = XInternAtom(dpy, "WM_PROTOCOLS", false);
  ATOM[WM_NAME] = XInternAtom(dpy, "WM_NAME", false);
  ATOM[WM_DELETE_WINDOW] = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
  ATOM[WM_STATE] = XInternAtom(dpy, "WM_STATE", false);
  ATOM[WM_TAKE_FOCUS] = XInternAtom(dpy, "WM_TAKE_FOCUS", false);
  ATOM[NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", false);
  ATOM[NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", false);
  ATOM[NET_WM_NAME] = XInternAtom(dpy, "_NET_WM_NAME", false);
  ATOM[NET_WM_WINDOW_OPACITY] = 
    XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", false);
  ATOM[NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false);
  ATOM[NET_WM_STATE_FULLSCREEN] = 
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false);
  ATOM[NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false);
  ATOM[NET_WM_WINDOW_TYPE_DIALOG] = 
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false);
  ATOM[NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", false);
  ATOM[NET_NUMBER_OF_DESKTOPS] = 
    XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false);
  ATOM[NET_WM_DESKTOP] = XInternAtom(dpy, "_NET_WM_DESKTOP", false);
  ATOM[NET_CURRENT_DESKTOP] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false);
  ATOM[NET_SHOWING_DESKTOP] = XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false);
}

Atom atom(const enum prop PROP) {
  return ATOM[PROP];
}

void addprop_clientlist(const Window W) {
  XChangeProperty(dpy, rootw, ATOM[NET_CLIENT_LIST], XA_WINDOW, 32, 
    PropModeAppend, (const unsigned char*) &W, 1);
}

void clrprop_clientlist() {
  XDeleteProperty(dpy, rootw, ATOM[NET_CLIENT_LIST]);
}

void delprop_actwindow(const Window W) {
  XDeleteProperty(dpy, W, ATOM[NET_ACTIVE_WINDOW]);
}

void setprop_nwks(const int N) {
  XChangeProperty(dpy, rootw, ATOM[NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, 
    PropModeReplace, (const unsigned char*) &N, 1);
}

void setprop_wks(const int N) {
  XChangeProperty(dpy, rootw, ATOM[NET_CURRENT_DESKTOP], XA_CARDINAL, 32,
    PropModeReplace, (const unsigned char*) &N, 1);
}

//////////////////////////////////////////////////////////////////
void app_prop(const Window W, const Atom PROP) {
  XChangeProperty(dpy, rootw, PROP, XA_WINDOW, 32, PropModeAppend, 
    (const unsigned char*) &W, 1);
}

void del_prop(const Window W, const Atom PROP) {
  XDeleteProperty(dpy, W, PROP);
}
  
void del_rootprop(const Atom PROP) {
  XDeleteProperty(dpy, rootw, PROP);
}
