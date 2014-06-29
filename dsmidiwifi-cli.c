#include <pthread.h>
#include <getopt.h>

#include <jack/jack.h>

#include "midi2udpthread.h"
#include "udp2midithread.h"

snd_seq_t *seq_handle;
int initSeq();
int midi_in_port;
int midi_out_port;

int jack = 0;
jack_port_t *output_port;
jack_port_t *input_port;
jack_client_t *jack_client;
jack_nframes_t nframes;
unsigned char output_midimsg[MIDI_MESSAGE_LENGTH];

int main(int argc, char *argv[]) {
	pthread_t midi2udpthread;
	pthread_t udp2midithread;


	static struct option long_options[] = {
		{"jack", 0, NULL, 'j'},
		{0, 0, 0, 0}
	};
	opterr = 0;
	if (getopt_long(argc, argv, "j", long_options, NULL) == 'j');
		jack = 1;


	if (initSeq() != 0) {
		fprintf(stderr, "Error initializing midi!");
	}

	//if (midi2udp_init() == 0) {
	//	printf("midi2udp initialized\n");
	//} else {
	//	fprintf(stderr, "Error initializing midi2udp!");
	//	return 1;
	//}
	
	if (udp2midi_init() == 0) {
		printf("udp2midi initialized\n");
	} else {
		fprintf(stderr, "Error initializing udp2midi!");
		return 1;
	}


	//pthread_create(&midi2udpthread, NULL, midi2udpthread_run, NULL);
	pthread_create(&udp2midithread, NULL, udp2midithread_run, NULL);

	//pthread_join(midi2udpthread, NULL);
	pthread_join(udp2midithread, NULL);
	return 0;
}

static int process(jack_nframes_t nframes, void *arg)
{
	int i;
	void* port_buf = jack_port_get_buffer(output_port, nframes);
	jack_midi_clear_buffer(port_buf);
	/*memset(buffer, 0, nframes*sizeof(jack_default_audio_sample_t));*/

	if (output_midimsg[0] == 0 && output_midimsg[1] == 0 && output_midimsg[2] == 0)
		return 0;


	//for(i=0; i<nframes; i++)
	//{
		jack_midi_event_write(port_buf, 0, output_midimsg, MIDI_MESSAGE_LENGTH);
/*		printf("wrote a note on, port buffer = 0x%x, event buffer = 0x%x\n", port_buf, buffer);*/
	//}
	output_midimsg[0] = 0;
	output_midimsg[1] = 0;
	output_midimsg[2] = 0;

	return 0;
}


int initSeq() {
	char name[] = "dsmidiwifi-cli";
	char inname[] = "in";
	char outname[] = "out";

	if (jack == 1) {
		jack_client = jack_client_open(name, 0, NULL);
		if (jack_client == NULL) {
			fprintf(stderr, "Could not create JACK client.\n");
			exit(1);
		}
		jack_set_process_callback (jack_client, process, 0);
		input_port = jack_port_register(jack_client, inname, JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
		output_port = jack_port_register(jack_client, outname, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
		output_midimsg[0] = 0;
		output_midimsg[1] = 0;
		output_midimsg[2] = 0;
		jack_activate(jack_client);
	}

	else {
		if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    			printf("Error opening ALSA sequencer.\n");
    			return 1;
  		}
		snd_seq_set_client_name(seq_handle, name);


		// Initialize midi ports
		midi_in_port = snd_seq_create_simple_port(seq_handle, inname, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
	
		if (midi_in_port < 0) {
			printf("Error creating MIDI input port!\n");
		
			snd_seq_close(seq_handle);
			return 1;
		}

		midi_out_port = snd_seq_create_simple_port(seq_handle, outname, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);
	
		if (midi_out_port < 0) {
			printf("Error creating MIDI output port!\n");
		
			snd_seq_close(seq_handle);
			return 1;
		}
	}
	
	return 0;
}

//void freeSeq() {
//	if ( snd_seq_close(seq_handle) < 0 ) {
//		printf("Error closing socket!\n");
//	}
//}
