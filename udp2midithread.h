#ifndef _UDP2MIDITHREAD_
#define _UDP2MIDITHREAD_

#include <alsa/asoundlib.h>

#include "settings.h"

bool udp2midi_init();
		
void * udp2midithread_run(void *);
		

#endif
