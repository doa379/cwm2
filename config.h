#pragma once

#include <X11/Xutil.h>
#include <X11/XF86keysym.h>
#include <palette.h>

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
*/

static const char* WMNAME = "cwm2";
static const char* WMVER = "-0.0";
static const int NWKS = { 8 };
static const int WKSFG = { Gray1 };
static const int WKSBG = { Cyan40 };
static const int FG0 = { Gray1 };
static const int BG0 = { Cyan60 };
static const int FG1 = { Gray1 };
static const int BG1 = { Cyan10 };
static const int FG2 = { Gray1 };
static const int BG2 = { Cyan30 };
static const int ACTBDR = { RedAlt };
static const int INACTBDR = { Cyan };
static const int FTBDR = { Yellow };
static const int WINGAP = { 0 };
static const int MOVESTEP_PX = { 5 };
static const int BDR_PX = { 4 };
static const float HPAD_PERC = { 0.005 };
static const float VPAD_PERC = { 0.007 };
static const float WKS_PERC = { 0.04 };
static const float CLIENTS_PERC = { 0.78 };
static const float ROOT_PERC = { 0.2 };
static const float VPANEL_PERC = { 0.03 };
static const bool SLOPPY_FOCUS = { true };

typedef enum {
  // Declare pool of calls
  WKS0,
  WKS1,
  WKS2,
  WKS3,
  WKS4,
  WKS5,
  WKS6,
  WKS7,
  WKS8,
  WKS9,
  UNMAPALL,
  REMAPALL,
  KILL,
  SWFOCUS,
  PREVCLI,
  NEXTCLI,
  SELTOGGLE,
  SELCLEAR,
  MOVEUP,
  MOVEDOWN,
  MOVELEFT,
  MOVERIGHT,
  RESIZEVINC,
  RESIZEVDEC,
  RESIZEHDEC,
  RESIZEHINC,
  QUIT,
  SELECT,
  RESIZE,
  STATE,
} calls_enum;

typedef struct {
  int mod, key;
  union {
    calls_enum call;
    const char* cmd;
  };
} input_t;

static const input_t KBD[] = {
  { Mod4Mask, XK_1, { WKS1 } },
  { Mod4Mask, XK_2, { WKS2 } },
  { Mod4Mask, XK_3, { WKS3 } },
  { Mod4Mask, XK_4, { WKS4 } },
  { Mod4Mask, XK_5, { WKS5 } },
  { Mod4Mask, XK_6, { WKS6 } },
  { Mod4Mask, XK_7, { WKS7 } },
  { Mod4Mask, XK_8, { WKS8 } },
  { Mod4Mask, XK_9, { WKS9 } },
  { Mod4Mask, XK_0, { WKS0 } },
  { Mod4Mask, XK_u, { UNMAPALL } },
  { Mod4Mask, XK_v, { REMAPALL } },
  { Mod4Mask, XK_Tab, { SWFOCUS } },
  { Mod4Mask, XK_o, { PREVCLI } },
  { Mod4Mask, XK_p, { NEXTCLI } },
  { Mod4Mask, XK_space, { SELTOGGLE } },
  { Mod4Mask, XK_c, { SELCLEAR } },
  { Mod4Mask | ShiftMask, XK_k, { KILL } },
  { Mod4Mask | ShiftMask, XK_Up, { MOVEUP } },
  { Mod4Mask | ShiftMask, XK_Down, { MOVEDOWN } },
  { Mod4Mask | ShiftMask, XK_Left, { MOVELEFT } },
  { Mod4Mask | ShiftMask, XK_Right, { MOVERIGHT } },
  { Mod4Mask | ControlMask, XK_Up, { RESIZEVINC } },
  { Mod4Mask | ControlMask, XK_Down, { RESIZEVDEC } },
  { Mod4Mask | ControlMask, XK_Left, { RESIZEHDEC } },
  { Mod4Mask | ControlMask, XK_Right, { RESIZEHINC } },
  { Mod4Mask | ShiftMask | ControlMask, XK_q, { QUIT } },
  // Shell Bindings
  { Mod4Mask, XK_n, { .cmd = "notify-send \"Test Key\"" } },
  { Mod4Mask, XK_Escape, { .cmd = "dmenu_run" } },
  { Mod4Mask, XK_l, { .cmd = "slock" } },
  { Mod4Mask, XF86XK_Sleep, { .cmd = "slock & yyy M" } },
  { Mod4Mask, XK_c, { .cmd = "xconsole" } },
  { Mod4Mask, XK_d, { .cmd = "xclock" } },
  { Mod4Mask, XK_Return, { .cmd = "xterm" } },
};

static const input_t BTN[] = {
  // Mouse Bindings
  { 0, Button1, { SELECT } },
  { Mod4Mask, Button3, { RESIZE } },
};
