#pragma once

#include <X11/Xlib.h>

bool init_wm();
void deinit_wm();
void noop(const long, const long, const long);
void mapnotify(const long, const long, const long);
void unmapnotify(const long, const long, const long);
void clientmessage(const long, const long, const long);
void configurenotify(const long, const long, const long);
void maprequest(const long, const long, const long);
void keypress(const long, const long, const long);
void btnpress(const long, const long, const long);
void enternotify(const long, const long, const long);
void propertynotify(const long, const long, const long);
static void unfocus(const Window);
static void focus(const Window);
static void quit();
static void kill();
static void prev();
static void next();
