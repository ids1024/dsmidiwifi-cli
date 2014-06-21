#ifndef _MIDI2UDPTHREAD_
#define _MIDI2UDPTHREAD_

#include <alsa/asoundlib.h>

#include "settings.h"

int midi2udp_init();
		
void add_ip(char[]);

void * midi2udpthread_run();
		
#endif
