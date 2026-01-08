#pragma once

#include <X11/Xutil.h>
#include <X11/XF86keysym.h>

#include "src/palette.h"
#include "src/input.h"
#include "src/calls.h"
#include "src/usercalls.h"

/*
KeySym {
  XK_Escape,
  XK_F1,
  XK_F2,
  XK_F3,
  XK_F4,
  XK_F5,
  XK_F6,
  XK_F7,
  XK_F8,
  XK_F9,
  XK_F10,
  XK_F11,
  XK_F12,
  XK_grave,
  XK_1,
  XK_2,
  XK_3,
  XK_4,
  XK_5,
  XK_6,
  XK_7,
  XK_8,
  XK_9,
  XK_0,
  XK_minus,
  XK_equal,
  XK_BackSpace,
  XK_Tab,
  XK_q,
  XK_w,
  XK_e,
  XK_r,
  XK_t,
  XK_y,
  XK_u,
  XK_t,
  XK_o,
  XK_p,
  XK_bracketleft,
  XK_bracketright,
  XK_a,
  XK_s,
  XK_d,
  XK_f,
  XK_g,
  XK_h,
  XK_j,
  XK_k,
  XK_l,
  XK_z,
  XK_x,
  XK_c,
  XK_v,
  XK_b,
  XK_n,
  XK_m,
  XK_comma,
  XK_period,
  XK_space,
  XK_Left,
  XK_Up,
  XK_Down,
  XK_Right,
  XK_Return,
  XF86XK_Sleep,
}

Buttons {
  Left      = Button1,
  Middle    = Button2,
  Right     = Button3,
  Up        = Button4,
  Down      = Button5,
}

Modifiers {
  Mod1Mask,
  Mod2Mask,
  Mod3Mask,
  Mod4Mask,
  Mod5Mask,
  ShiftMask,
  ControlMask,
  LockMask
}
*/

char const WMNAME[] = "cwm2-0.1";
unsigned const num_wks = 4;
char const* font_name = "Fixed:size=12";
/* Values in px */
unsigned const bdrw = 2;
unsigned const gaps = 0;
unsigned const trayw = 100;
int const rootbg = palette_gray60;
int const COLORS[] = { 
/* Color Scheme { BG, ACT, SEL } */
  palette_cyan10,
  palette_cyan50,
  palette_yellow
};

input_t const KBD[] = {
  /* Keyboard bindings */
  { Mod4Mask, XK_Escape, .cmd = "dmenu_run" },
  { Mod4Mask, XK_t, .cmd = "xclock" },
  { Mod4Mask, XK_e, .cmd = "xeyes" },
  { Mod4Mask, XK_F1, .call = calls_wk1 },
  { Mod4Mask, XK_F2, .call = calls_wk2 },
  { Mod4Mask, XK_F3, .call = calls_wk3 },
  { Mod4Mask, XK_F4, .call = calls_wk4 },
  { Mod4Mask, XK_F5, .call = calls_wk5 },
  { Mod4Mask, XK_F6, .call = calls_wk6 },
  { Mod4Mask, XK_F7, .call = calls_wk7 },
  { Mod4Mask, XK_F8, .call = calls_wk8 },
  { Mod4Mask, XK_F9, .call = calls_wk9 },
  { Mod4Mask, XK_F10, .call = calls_wk10 },
  { Mod4Mask, XK_F11, .call = calls_wk11 },
  { Mod4Mask, XK_F12, .call = calls_wk12 },
  { Mod4Mask, XK_grave, .call = calls_cli_last },
  { Mod4Mask, XK_0, .call = calls_cli0 },
  { Mod4Mask, XK_1, .call = calls_cli1 },
  { Mod4Mask, XK_2, .call = calls_cli2 },
  { Mod4Mask, XK_3, .call = calls_cli3 },
  { Mod4Mask, XK_4, .call = calls_cli4 },
  { Mod4Mask, XK_5, .call = calls_cli5 },
  { Mod4Mask, XK_6, .call = calls_cli6 },
  { Mod4Mask, XK_7, .call = calls_cli7 },
  { Mod4Mask, XK_8, .call = calls_cli8 },
  { Mod4Mask, XK_9, .call = calls_cli9 },
  { Mod4Mask, XK_Tab, .call = calls_cli_next },
  { ShiftMask | Mod4Mask, XK_Tab, .call = calls_cli_prev },
  { Mod4Mask, XK_minus, .call = calls_cli_prev },
  { Mod4Mask, XK_equal, .call = calls_cli_next },
  { Mod4Mask, XK_r, .call = calls_cli_raise_toggle },
  { Mod4Mask, XK_g, .call = calls_grid_arrange },
  { Mod4Mask, XK_m, .call = calls_cli_mode_toggle },
  { Mod4Mask, XK_space, .call = calls_sel_toggle },
  { Mod4Mask, XK_c, .call = calls_sel_clear },
  { Mod4Mask, XK_grave, .call = calls_cli_last },
  { Mod4Mask, XK_Home, .call = calls_wk_prev },
  { Mod4Mask, XK_End, .call = calls_wk_next },
  { Mod4Mask, XK_Insert, .call = calls_wk_map },
  { Mod4Mask, XK_Delete, .call = calls_wk_unmap },
  { ShiftMask | Mod4Mask, XK_d, .call = calls_kill },
  { ShiftMask | Mod4Mask, XK_Home, 
    .call = calls_cli_wk_prev_move },
  { ShiftMask | Mod4Mask, XK_End,
    .call = calls_cli_wk_next_move },
  { ShiftMask | Mod4Mask, XK_F1,
    .call = calls_cli_wk1_move },
  { ShiftMask | Mod4Mask, XK_F2,
    .call = calls_cli_wk2_move },
  { ShiftMask | Mod4Mask, XK_F3,
    .call = calls_cli_wk3_move },
  { ShiftMask | Mod4Mask, XK_F4,
    .call = calls_cli_wk4_move },
  { ShiftMask | Mod4Mask, XK_F5,
    .call = calls_cli_wk5_move },
  { ShiftMask | Mod4Mask, XK_F6,
    .call = calls_cli_wk6_move },
  { ShiftMask | Mod4Mask, XK_F7,
    .call = calls_cli_wk7_move },
  { ShiftMask | Mod4Mask, XK_F8,
    .call = calls_cli_wk8_move },
  { ShiftMask | Mod4Mask, XK_F9,
    .call = calls_cli_wk9_move },
  { ShiftMask | Mod4Mask, XK_F10,
    .call = calls_cli_wk10_move },
  { ShiftMask | Mod4Mask, XK_F11,
    .call = calls_cli_wk11_move },
  { ShiftMask | Mod4Mask, XK_F12,
    .call = calls_cli_wk12_move },
  { ShiftMask | ControlMask | Mod4Mask, XK_q, 
    .call = calls_quit },
  { Mod4Mask, XK_z, .call = calls_debug },
};

input_t const BTN[] = {
  /* Mouse Bindings */
  { Mod4Mask, Button1, .call = calls_cli_move },
  { Mod4Mask, Button3, .call = calls_cli_resize },
};

size_t const kbdlen = sizeof KBD / sizeof KBD[0];
size_t const btnlen = sizeof BTN / sizeof BTN[0];
