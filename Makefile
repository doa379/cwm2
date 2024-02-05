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
CC_FLAGS = -std=c2x -Wall -fPIE -fPIC -pedantic
REL_CFLAGS = -O3
REL_LDFLAGS = -s
#DBG_CFLAGS = -O1 -g -fsanitize=address -fno-omit-frame-pointer
#DBG_LDFLAGS = -g -fsanitize=address
DBG_CFLAGS = -O1 -g -fno-omit-frame-pointer
DBG_LDFLAGS =

CFLAGS = $(REL_CFLAGS)
LDFLAGS = $(REL_LDFLAGS)
EXEC = cwm2.bin
DBG_EXEC = cwm2~dbg.bin

.if "$(DEBUG)" == "1"
  CFLAGS = $(DBG_CFLAGS)
  LDFLAGS = $(DBG_LDFLAGS)
  EXEC = $(DBG_EXEC)
.endif

SRC = src/msg.c src/main.c src/events.c src/lib.c src/panel.c src/wm.c
OBJ = $(SRC:.c=.o)

.POSIX:
all: $(EXEC)

.SUFFIXES: .c .o
.c.o: config.h
	@echo Build $< "-->" $@ ...
	@$(CC) $(CC_FLAGS) -c $(CFLAGS) $(INCS) $< -o $@

$(EXEC): $(OBJ)
	@echo Linking...
	@$(CC) $(CC_FLAGS) $(LIBSPATH) $(LIBS) $(LDFLAGS) $(OBJ) -o $@
	@echo $(EXEC)

clean:
	@echo Cleaning...
	rm -f $(OBJ)
	rm -f $(EXEC) $(DBG_EXEC) *.tmp *.core
