INCS=-I src \
	-I /usr/X11R6/include \
  -I /usr/include \
  -I /usr/include/freetype2 \
  -I /usr/local/include/freetype2 \
  -I /usr/lib/dbus-1.0/include \
  -I /usr/local/lib/dbus-1.0/include \
  -I /usr/include/dbus-1.0 \
  -I /usr/local/include/dbus-1.0
LIBSPATH= -L . -Wl,-R . '-Wl,-R$$ORIGIN' \
  -L /usr/X11R6/lib \
  -L /usr/lib \
  -L /usr/lib64 \
  -L /usr/local/lib

LIBS=-l X11 -l Xinerama -l dbus-1 -l Xft -l m

CC=cc
FLAGS=-std=c2x -Wall -fPIE -fPIC -pedantic

REL_CFLAGS=-O3
DBG_CFLAGS=-O1 -g -fno-omit-frame-pointer
REL_LFLAGS=-s
DBG_LFLAGS=
REL=cwm2.bin
DBG=cwm2~dbg.bin

HDR_CONFIG=config.h
HDR_SRC=
SRC=main.c \
	clr.c \
	font.c \
	root.c \
	mon.c \
	wm.c \
	wk.c \
	cli.c \
	ev.c \
	input.c \
	calls.c \
	wg.c \
	panel.c \
	tray.c \
	arrange.c \
	prop.c \
	mascot.c \
	clib.c \
	cblk.c \
	cstring.c
