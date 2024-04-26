LOCAL = .
INCS = -I /usr/X11R6/include \
  -I /usr/include \
  -I /usr/local/include \
  -I /usr/lib/dbus-1.0/include \
  -I /usr/local/lib/dbus-1.0/include \
  -I /usr/include/dbus-1.0 \
  -I /usr/local/include/dbus-1.0 \
  -I $(LOCAL)/src
LIBSPATH = -L $(LOCAL)/ -Wl,-R$(LOCAL)/ '-Wl,-R$$ORIGIN' \
  -L /usr/X11R6/lib \
  -L /usr/lib \
  -L /usr/lib64 \
  -L /usr/local/lib

LIBS = -l X11 -l Xinerama -l dbus-1

CC = clang
FLAGS = -std=c2x -Wall -fPIE -fPIC -pedantic
#DBG_CFLAGS = -O1 -g -fsanitize=address -fno-omit-frame-pointer
#DBG_LDFLAGS = -g -fsanitize=address

CFLAGS = -O3
LDFLAGS = -s
EXEC = cwm2.bin

.if "$(DEBUG)" == "1"
  CFLAGS = -O1 -g -fno-omit-frame-pointer
  LDFLAGS =
  EXEC = cwm2~dbg.bin
.endif

SRC = src/main.c src/Xlib.c src/wm.c src/atoms.c src/ev.c src/lib.c src/draw.c
OBJ = $(SRC:.c=.o)

.POSIX:
all: $(EXEC)

.SUFFIXES: .c .o
.c.o: config.h
	@echo Build $< "-->" $@ ...
	@$(CC) $(FLAGS) -c $(CFLAGS) $(INCS) $< -o $@

$(EXEC): $(OBJ)
	@echo Linking...
	@$(CC) $(FLAGS) $(LIBSPATH) $(LIBS) $(LDFLAGS) $(OBJ) -o $@
	@echo $(EXEC)

clean:
	@echo Cleaning...
	rm -f $(OBJ)
	rm -f $(EXEC) $(DBG_EXEC) *.tmp *.core
