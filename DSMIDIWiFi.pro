TEMPLATE = app
TARGET += 
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lasound
QT += network
QT -= gui

# Input
HEADERS += midi2udpthread.h settings.h udp2midithread.h
SOURCES += dswifimidi.cpp midi2udpthread.cpp udp2midithread.cpp
