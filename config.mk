PREFIX = /usr/local

CXXFLAGS+= -fPIE
INCPATH +=
LDFLAGS += -lasound -lpthread

#CC = cc
#CC = musl-gcc
#CC = clang
