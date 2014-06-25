#include <pthread.h>

#include "midi2udpthread.h"
#include "udp2midithread.h"

snd_seq_t *seq_handle;
int initSeq();
int midi_in_port;
int midi_out_port;


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



int initSeq() {
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    		printf("midi2udp: Error opening ALSA sequencer.\n");
    		return 1;
  	}
	snd_seq_set_client_name(seq_handle, "dsmidiwifi-cli");


	// Initialize midi ports
	midi_in_port = snd_seq_create_simple_port(seq_handle, "in", SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
	
	if (midi_in_port < 0) {
		printf("midi2udp: Error creating MIDI port!\n");
		
		snd_seq_close(seq_handle);
		return 1;
	}

	midi_out_port = snd_seq_create_simple_port(seq_handle, "out", SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);
	
	if (midi_out_port < 0) {
		printf("udp2midi: Error creating MIDI port!\n");
		
		snd_seq_close(seq_handle);
		return 1;
	}

	
	return 0;
}

//void freeSeq() {
//	if ( snd_seq_close(seq_handle) < 0 ) {
//		printf("Error closing socket!\n");
//	}
//}
