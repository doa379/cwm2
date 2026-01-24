#pragma once

#include <X11/Xlib.h>

int evcalls_configure_request(Window const, int const,
int const, int const, int const);
void evcalls_configure_notify(Window const, int const,
int const, int const, int const);
void evcalls_map_override_redirect(Window const, int const,
int const, int const, int const);
void evcalls_map_request(Window const, int const, int const,
int const, int const);
void evcalls_destroy_notify(Window const);
void evcalls_motion_notify(Window const, int const, 
int const, int const, int const);
void evcalls_key_press(unsigned const, unsigned const);
void evcalls_btn_press(Window const, unsigned const, 
unsigned const, int const, int const, int const, int const);
void evcalls_enter_notify(Window const);
void evcalls_leave_notify(Window const);
void evcalls_focus_change(Window const);
void evcalls_property_notify(Window const);
void evcalls_expose(Window const);
void evcalls_byte_msg(Window const, Atom const, 
char const[]);
void evcalls_short_msg(Window const, Atom const, 
short const[]);
void evcalls_long_msg(Window const, Atom const, 
long const[]);
