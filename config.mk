PREFIX = /usr/local

CXXFLAGS+= -fPIE
INCPATH += -isystem /usr/include/qt -isystem /usr/include/qt/QtNetwork -isystem /usr/include/qt/QtCore
LDFLAGS += -lasound -lQt5Network -lQt5Core -lpthread

#CC = cc
#CC = musl-gcc
#CC = clang
