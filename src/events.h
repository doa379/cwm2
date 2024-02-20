#pragma once

#include <X11/Xlib.h>
#include <signal.h>

void events(Display*, volatile sig_atomic_t*);
