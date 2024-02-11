#pragma once

#include <X11/Xlib.h>
#include <signal.h>

void events(Display*, const Window, volatile sig_atomic_t*);
