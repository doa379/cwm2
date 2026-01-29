INCS=-I . \
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

DBG_CFLAGS=-O1 -g -fno-omit-frame-pointer
DBG_LFLAGS=

CFLAGS=${DBG_CFLAGS}

SRC0=main.c
BIN0=main.bin
SRC1=../src/cblk.c cblk.c
BIN1=cblk.bin
SRC1=atom_name.c
BIN1=atom_name.bin
