INCS=-I src \
  -I /usr/include \
  -I /usr/include/freetype2 \
  -I /usr/local/include \
  -I /usr/local/include/freetype2

LIBSPATH= -L . -Wl,-R . '-Wl,-R$$ORIGIN' \
  -L /usr/lib \
  -L /usr/lib64 \
  -L /usr/local/lib

LIBS=-l X11 -l Xinerama -l Xft -l m

CC=cc
FLAGS=-Wall -fPIE -fPIC -pedantic

REL_CFLAGS=-O3
REL_LFLAGS=-s
REL_BIN=cwm2.bin

DBG_CFLAGS=-O1 -g -fno-omit-frame-pointer
DBG_LFLAGS=
DBG_BIN=cwm2~dbg.bin

# Compiler optim'n to be set manually :/
CFLAGS=${REL_CFLAGS}
#CFLAGS=${DBG_CFLAGS}

HDR_CONFIG=config.h
SRC=src/main.c \
  src/clr.c \
  src/font.c \
  src/root.c \
  src/mon.c \
  src/wm.c \
  src/wk.c \
  src/cli.c \
  src/ev.c \
  src/evcalls.c \
  src/input.c \
  src/calls.c \
  src/usercalls.c \
  src/wg.c \
  src/panel.c \
  src/status.c \
  src/tray.c \
  src/arrange.c \
  src/prop.c \
  src/mascot.c \
  src/clib.c \
  src/cvec.c \
  src/cblk.c \
  src/cstring.c
