#ifndef _MIDI2UDPTHREAD_
#define _MIDI2UDPTHREAD_

#include <set>
#include <string>

#include <alsa/asoundlib.h>

#include "settings.h"

using namespace std;

bool midi2udp_init();
		
void add_ip(string ip);

void * midi2udpthread_run(void *);
		
#endif
