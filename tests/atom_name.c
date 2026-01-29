#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

int main(int const argc, char const* argv[]) {
  Display *dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    printf("Unable to open display\n");
    return -1;
  }

  long val = atol(argv[1]);
  char* atom_name = XGetAtomName(dpy, val);
  if (atom_name) {
    printf("Atom name for %ld: %s\n", val, atom_name);
    XFree(atom_name);
  } else {
    printf("No atom found for value %ld\n", val);
  }

  XCloseDisplay(dpy);
  return 0;
}
