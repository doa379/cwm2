#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdbool.h>

#include "input.h"

extern Display* dpy;

extern input_t const KBD[];
extern input_t const BTN[];
extern size_t const kbdlen;
extern size_t const btnlen;

static unsigned numlockmask;

static void 
input_numlockmask_update(void) {
	XModifierKeymap* modmap = XGetModifierMapping(dpy);
	for (unsigned i = 0; i < 8; i++)
		for (unsigned j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod +
            j] == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);

  XFreeModifiermap(modmap);
}

static unsigned 
input_cleanmask(unsigned const mask) {
  return mask & ~(numlockmask | LockMask) & 
      (ShiftMask |
       ControlMask |
       Mod1Mask |
       Mod2Mask |
       Mod3Mask |
       Mod4Mask |
       Mod5Mask);
}

void 
input_keys_grab(Window const win) {
  input_numlockmask_update();
	unsigned const modifiers[] = {
    0, LockMask, numlockmask, numlockmask | LockMask
  };

	XUngrabKey(dpy, AnyKey, AnyModifier, win);
	int start;
  int end;
	XDisplayKeycodes(dpy, &start, &end);
  int skip;
	KeySym* syms = XGetKeyboardMapping(dpy, 
    start, end - start + 1, &skip);

	if (syms == NULL)
		return;

	for (int k = start; k <= end; k++)
    for (size_t i = 0; i < kbdlen; i++)
      if (KBD[i].sym == syms[(k - start) * skip])
 				for (int j = 0; j < 4; j++)
 					XGrabKey(dpy, k, KBD[i].mod | modifiers[j], 
            win, True, GrabModeAsync, GrabModeAsync);

 	XFree(syms);
}

void
input_btns_grab(Window const win) {
  input_numlockmask_update();
	unsigned const modifiers[] = {
    0, LockMask, numlockmask, numlockmask | LockMask
  };

  XUngrabButton(dpy, AnyButton, AnyModifier, win);
  for (size_t i = 0; i < btnlen; i++)
    for (int j = 0; j < 4; j++)
      XGrabButton(dpy, BTN[i].sym, 
        BTN[i].mod | modifiers[j],
          win, False, ButtonPressMask | ButtonReleaseMask, 
            GrabModeAsync, GrabModeSync, None, None);
}

void
input_btns_ungrab(Window const win) {
  XUngrabButton(dpy, AnyButton, AnyModifier, win);
  XGrabButton(dpy, AnyButton, AnyModifier, win,
    False, ButtonPressMask | ButtonReleaseMask, 
      GrabModeSync, GrabModeSync, None, None);
}

input_t const* 
input_key(unsigned const mask, unsigned const code) {
  unsigned const mod = input_cleanmask(mask);
  for (size_t i = 0; i < kbdlen; i++)
    if (input_cleanmask(KBD[i].mod) == mod && 
        XKeysymToKeycode(dpy, KBD[i].sym) == code)
      return &KBD[i];
  
  return NULL;
}

input_t const* 
input_btn(unsigned const state, unsigned const code) {
  unsigned const mod = input_cleanmask(state);
  for (size_t i = 0; i < btnlen; i++)
    if (input_cleanmask(BTN[i].mod) == mod &&
          BTN[i].sym == code)
      return &BTN[i];
  
  return NULL;
}
