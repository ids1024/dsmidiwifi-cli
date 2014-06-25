#include <pthread.h>

#include "midi2udpthread.h"
#include "udp2midithread.h"

snd_seq_t *seq_handle;

int initSeq() {
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    	printf("midi2udp: Error opening ALSA sequencer.\n");
    	return 1;
  	}
	
	snd_seq_set_client_name(seq_handle, "dsmidiwifi-cli");
	return 0;
}

void freeSeq() {
	if ( snd_seq_close(seq_handle) < 0 ) {
		printf("Error closing socket!\n");
	}
}

int main() {
	pthread_t midi2udpthread;
	pthread_t udp2midithread;

	if (initSeq() != 0) {
		fprintf(stderr, "Error initializing midi!");
	}

	if (midi2udp_init() == 0) {
		printf("midi2udp initialized\n");
	} else {
		fprintf(stderr, "Error initializing midi2udp!");
		return 1;
	}
	
	if (udp2midi_init() == 0) {
		printf("udp2midi initialized\n");
	} else {
		fprintf(stderr, "Error initializing udp2midi!");
		return 1;
	}


	pthread_create(&midi2udpthread, NULL, midi2udpthread_run, NULL);
	pthread_create(&udp2midithread, NULL, udp2midithread_run, NULL);

	pthread_join(midi2udpthread, NULL);
	pthread_join(udp2midithread, NULL);
	return 0;
}
