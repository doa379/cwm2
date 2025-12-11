#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "input.h"
#include "calls.h"

extern Display* dpy;
extern input_t const KBD[];
extern input_t const BTN[];
extern size_t const kbdlen;
extern size_t const btnlen;
static unsigned numlockmask;

static void input_update_numlockmask(void) {
	XModifierKeymap* modmap = XGetModifierMapping(dpy);
	for (unsigned i = 0; i < 8; i++)
		for (unsigned j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);

  XFreeModifiermap(modmap);
}

static unsigned input_cleanmask(unsigned const mask) {
  return mask & ~(numlockmask | LockMask) & 
      (ShiftMask |
       ControlMask |
       Mod1Mask |
       Mod2Mask |
       Mod3Mask |
       Mod4Mask |
       Mod5Mask);
}

void input_keys_grab(long const win) {
  input_update_numlockmask();
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
            win, true, GrabModeAsync, GrabModeAsync);

 	XFree(syms);
}

void input_btns_grab(long const win) {
  /* Call this func when win gains focus */
  input_update_numlockmask();
	unsigned const modifiers[] = {
    0, LockMask, numlockmask, numlockmask | LockMask
  };

  for (size_t i = 0; i < btnlen; i++)
    for (int j = 0; j < 4; j++)
      XGrabButton(dpy, BTN[i].sym, 
        BTN[i].mod | modifiers[j],
          win, false, ButtonPressMask | ButtonReleaseMask, 
            GrabModeAsync, GrabModeSync, None, None);
}

void input_btns_ungrab(long const win) {
  /* Call this func when win loses focus */
  XUngrabButton(dpy, AnyButton, AnyModifier, win);
  XGrabButton(dpy, AnyButton, AnyModifier, win,
    false, ButtonPressMask | ButtonReleaseMask, 
      GrabModeSync, GrabModeSync, None, None);
}

void input_key(unsigned const mask, unsigned const code) {
  unsigned const mod = input_cleanmask(mask);
  for (size_t i = 0; i < kbdlen; i++)
    if (input_cleanmask(KBD[i].mod) == mod && 
        XKeysymToKeycode(dpy, KBD[i].sym) == code) {
      if (KBD[i].cmd) {
        if (fork() == 0) {
          close(ConnectionNumber(dpy));
          char* const args[] = { NULL };
          execvp(KBD[i].cmd, args);
        }
      } else if (KBD[i].call)
          KBD[i].call();

      break;
    }
}

void input_btn(unsigned const mask, unsigned const code) {
  unsigned const mod = input_cleanmask(mask);
  for (size_t i = 0; i < btnlen; i++)
    if (input_cleanmask(BTN[i].mod) == mod &&
        XKeysymToKeycode(dpy, BTN[i].sym) == code) {
      BTN[i].call();
      break;
    }
}
