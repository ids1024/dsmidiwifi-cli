include config.mk

SRC = dswifimidi.cpp midi2udpthread.cpp udp2midithread.cpp
OBJ = ${SRC:.c=.o}

all: dsmidiwifi-cli

${OBJ}: config.mk

dsmidiwifi-cli: ${OBJ}
	@echo CC -o $@
	@${CXX} -o $@ ${OBJ} ${CXXFLAGS} $(INCPATH) ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f dsmidiwifi ${OBJ}


install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dsmidiwifi-cli ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dsmidiwifi-cli

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/dsmidiwifi-cli

.PHONY: all clean install uninstall
