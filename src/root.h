#pragma once

#include <X11/Xlib.h>

int root_init(void);
void root_deinit(void);
int root_wins_init(void);
void root_wins_deinit(void);
void root_query(void);
void root_ev_enqueue(void);
int root_stack_dist(Window);
Window root_ptr_query(int* const, int* const);
