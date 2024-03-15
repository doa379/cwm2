#pragma once

#include <X11/Xlib.h>

bool init_wm();
void deinit_wm();
void init_wks();
void noop(const long, const long, const long);
void mapnotify(const long, const long, const long);
void unmapnotify(const long, const long, const long);
//void clientmessage(const long, const long, const long);
void configurenotify(const long, const long, const long);
void maprequest(const long, const long, const long);
void keypress(const long, const long, const long);
void btnpress(const long, const long, const long);
void enternotify(const long, const long, const long);
void propertynotify(const long, const long, const long);

void switch_wks(const long, const long, const long);
void msg1(const long, const long, const long);
void msg2(const long, const long, const long);

void unfocus(const Window);
void focus(const Window);
void refresh_panel();
void wks0();
void wks1();
void wks2();
void wks3();
void wks4();
void wks5();
void wks6();
void wks7();
void wks8();
void wks9();
void kill();
void prev();
void next();
void quit();
