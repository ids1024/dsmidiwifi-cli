include config.mk

SRC = dsmidiwifi-cli.c udp2midithread.c
OBJ = ${SRC:.c=.o}

all: dsmidiwifi-cli

${OBJ}: config.mk

dsmidiwifi-cli: ${OBJ}
	@echo CXX -o $@
	@${CXX} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f dsmidiwifi-cli ${OBJ}


install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dsmidiwifi-cli ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dsmidiwifi-cli

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/dsmidiwifi-cli

.PHONY: all clean install uninstall
