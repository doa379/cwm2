#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdio.h>

extern Display* dpy;
static char BUF[32];

static char const* prop_text(Window const win, Atom atom) {
  BUF[0] = '\0';
  XTextProperty prop;
  if (XGetTextProperty(dpy, win, &prop, atom) && 
      prop.nitems) {
    char** list = NULL;
    int n = 0;
 	  if (prop.encoding == XA_STRING)
 		  strncpy(BUF, (char*) prop.value, sizeof BUF - 1);
 	  else if (XmbTextPropertyToTextList(dpy, &prop, &list, &n)
      && n > 0 && *list) {
 		  strncpy(BUF, *list, sizeof BUF - 1);
 		  XFreeStringList(list);
 	  }
 	
    BUF[sizeof BUF - 1] = '\0';
 	  XFree(prop.value);
  }

  return BUF;
}

char const* prop_root(void) {
  if (prop_text(DefaultRootWindow(dpy), XA_WM_NAME)[0] ==
      '\0')
 	  strcpy(BUF, "status");

  return BUF;
}
 
char const* prop_name(Window const win) {
  if (prop_text(win, XA_WM_NAME)[0] == '\0')
 	  prop_text(win, XInternAtom(dpy, "_NET_WM_NAME", False));

  return BUF;
}

char const* prop_ico(Window const win) {
  BUF[0] = '\0';
  XTextProperty prop;
  if (XGetWMIconName(dpy, win, &prop) && prop.nitems > 0) {
    strncpy(BUF, (char*) prop.value, sizeof BUF - 1);
    BUF[sizeof BUF - 1] = '\0';
    XFree(prop.value);
  }

  return BUF;
}
      
void prop_(Window const win, Atom const prop) {
  int result;
  Atom actual_type;
  int actual_format;
  unsigned long bytes_after;
  unsigned char* data;
  unsigned long nitems;

  /*Atom prop = XInternAtom(dpy, "_NET_WM_ICON", False);*/
  if (XGetWindowProperty(dpy, win, 
      prop, 
      0, ~0L, 
      False,
      XA_CARDINAL /*AnyPropertyType*/,
      &actual_type, 
      &actual_format,
      &nitems, 
      &bytes_after,
      &data)) {
    fprintf(stdout, "Success\n");
    fprintf(stdout, "Data %s\n", data);
    XFree(data);
  }
}
      
void prop_hints(Window const win, GC const gc) {
  XWMHints* wm_hints = XGetWMHints(dpy, win);
  if (wm_hints) {
    if (wm_hints->flags & IconPixmapHint) {
      fprintf(stdout, "Icon Pixmap available\n");
      /*
         Pixmap p = XCreateBitmapFromData(dpy, 
         c->parwin, 
         (char*) wm_hints->ico_pixmap, 
         units_clen(), units_clen());
         */
      XCopyPlane(dpy, wm_hints->icon_pixmap, 
          win, gc, 0, 0, 200, 200, 0, 0, 1);
    }

    XFree(wm_hints);
  }
}

int prop_sendmsg(Window const win, Atom const atom) {
  XEvent dummy;
  dummy.type = ClientMessage;
  dummy.xclient.type = ClientMessage;
  dummy.xclient.serial = 0;
  dummy.xclient.send_event = True;
  dummy.xclient.display = dpy;
  dummy.xclient.window = win;
  dummy.xclient.message_type = atom;
    /*XInternAtom(dpy, "DUMMY_EVENT", False);*/
    /*XInternAtom(dpy, "WM_PROTOCOLS", False);*/
  dummy.xclient.format = 32;
  dummy.xclient.data.l[0] = atom;
  dummy.xclient.data.l[1] = CurrentTime;
  int const mask = NoEventMask | (1L << 17);
  Status const status = 
    XSendEvent(dpy, win, False, mask, &dummy);
  XSync(dpy, False);
  return status;
}
