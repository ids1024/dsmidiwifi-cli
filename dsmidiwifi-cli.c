#include <pthread.h>

#include "midi2udpthread.h"
#include "udp2midithread.h"


int main()
{
	pthread_t midi2udpthread;
	pthread_t udp2midithread;

	int res = midi2udp_init();
	pthread_create(&midi2udpthread, NULL, midi2udpthread_run, NULL);
	if(res == 0) {
		printf("midi2udp initialized\n");
	} else {
		fprintf(stderr, "Error initializing midi2udp!");
		return 1;
	}
	
	res = udp2midi_init();
	if(res == 0) {
		printf("udp2midi initialized\n");
	pthread_create(&udp2midithread, NULL, udp2midithread_run, NULL);
	} else {
		fprintf(stderr, "Error initializing udp2midi!");
		return 1;
	}

	pthread_join(midi2udpthread, NULL);
	pthread_join(udp2midithread, NULL);
	return 0;
}
