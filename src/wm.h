#pragma once

#include <X11/Xlib.h>

typedef struct client_s client_t;

bool init_wks();
void deinit_wks();
bool init_monitors();
void deinit_monitors();
void init_wm();
void deinit_wm();
void noop(const long, const long, const long);
void mapnotify(const long, const long, const long);
void unmapnotify(const long, const long, const long);
//void clientmessage(const long, const long, const long);
void configureroot(const long, const long, const long);
void configurenotify(const long, const long, const long);
void maprequest(const long, const long, const long);
void motionnotify(const long, const long, const long);
void keypress(const long, const long, const long);
void btnpress(const long, const long, const long);
void enternotify(const long, const long, const long);
void propertynotify(const long, const long, const long);

void switch_wks(const long, const long, const long);

void unfocus(const Window);
void focus(const Window);
void refresh_panel();
client_t* prev_client(client_t*, const bool);
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
void mon0();
void mon1();
void mon2();
void mon3();
void mon4();
void mon5();
void mon6();
void mon7();
void mon8();
void mon9();
void kill();
void mode();
void prev();
void next();
void quit();
