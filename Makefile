include config.mk

SRC = udp2midi.c
OBJ = ${SRC:.c=.o}

all: udp2midi

${OBJ}: config.mk

udp2midi: ${OBJ}
	@echo CXX -o $@
	@${CXX} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f udp2midi ${OBJ}


install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f udp2midi ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/udp2midi

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/udp2midi

.PHONY: all clean install uninstall
