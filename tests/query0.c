#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  Display* dpy = XOpenDisplay(NULL);
  Window root = DefaultRootWindow(dpy);

  while (1) {
    Window rr;
    Window cr;
    int rx;
    int ry;
    int wx;
    int wy;
    unsigned mask;

    XQueryPointer(dpy, root, &rr, &cr, &rx, &ry, &wx, &wy, &mask);
    fprintf(stdout, "mask = %u\n", mask);
    usleep(500000);
  }

  return 0;
}

