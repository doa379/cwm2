include config.mk

OBJ=$(SRC:.c=.o)
FLAG=Release

all: release

.c.o:
	@echo CC	$@
	@${CC} ${FLAGS} ${CFLAGS} ${INCS} $< -c -o $@

${OBJ}: ${HDR_CONFIG} config.mk

release: ${OBJ}
	@echo Linking ${FLAG}...
	@${CC} ${FLAGS} ${LIBSPATH} ${LIBS} ${REL_LFLAGS} $? \
    -o ${REL_BIN}
	@echo ${REL_BIN}

debug: FLAG=Debug CFLAGS=${DBG_CFLAGS}
debug: ${OBJ}
	@echo Linking ${FLAG}...
	@${CC} ${FLAGS} ${LIBSPATH} ${LIBS} ${DBG_LFLAGS} $? \
    -o ${DBG_BIN}
	@echo ${DBG_BIN}

install: all
	cp -f ${REL_BIN} /usr/local/bin
	chmod 755 /usr/local/bin/${REL_BIN}

uninstall:
	rm -f /usr/local/bin/${REL_BIN}

clean:
	@echo Cleaning...
	rm -f ${OBJ} ${REL_BIN} ${DBG_BIN}

.SUFFIXES: .c .o
.PHONY: all clean install uninstall
