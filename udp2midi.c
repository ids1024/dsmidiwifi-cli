#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <jack/jack.h>

#include "settings.h"

int initSeq();
int midi_in_port;
int midi_out_port;

jack_port_t *output_port;
jack_port_t *input_port;
jack_client_t *jack_client;
jack_nframes_t nframes;
unsigned char output_midimsg[MIDI_MESSAGE_LENGTH];
unsigned char udp2midi_midimsg[MIDI_MESSAGE_LENGTH];

int udp2midi_init();
		
int main(int argc, char *argv[]) {
	struct sockaddr_in serv_addr;
	int udp_socket;


	if (initSeq() != 0) {
		fprintf(stderr, "Error initializing midi!");
	}


		
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PC_PORT);

	if (bind(udp_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		char msg[256];
		sprintf(msg, "Could not bind to port %d!\n", PC_PORT);
		printf(msg);
		exit(1);
	}

	while (1) {
		struct sockaddr_in from_address;
		socklen_t from_address_len;
		ssize_t res;

		// Receive from UDP
		from_address_len = sizeof(from_address);
		res = recvfrom(udp_socket, &udp2midi_midimsg, MIDI_MESSAGE_LENGTH, 0, (struct sockaddr *) &from_address, &from_address_len);
		if ( res > MIDI_MESSAGE_LENGTH) {
			printf("udp2midi: received a message of %d bytes, but max length is %d byte\n", res, MIDI_MESSAGE_LENGTH);
		} else if ( res == -1 ) {
			printf("udp2midi: Error receiving data!\n");
		}
			
		if ((udp2midi_midimsg[0] == 0) && (udp2midi_midimsg[1] == 0) && (udp2midi_midimsg[2] == 0)) {

			char from_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &from_address.sin_addr, from_ip, INET_ADDRSTRLEN);
			printf("Keepalive from: %s\n", from_ip);
		
		} else {

			// Send to MIDI
			printf("udp2midi: Sending event: 0x%x 0x%x 0x%x\n", udp2midi_midimsg[0], udp2midi_midimsg[1], udp2midi_midimsg[2]);
	
			output_midimsg[0] = udp2midi_midimsg[0];
			output_midimsg[1] = udp2midi_midimsg[1];
			output_midimsg[2] = udp2midi_midimsg[2];

		}
	}




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
	char name[] = "udp2midi";
	char outname[] = "out";

	jack_client = jack_client_open(name, 0, NULL);
	if (jack_client == NULL) {
		fprintf(stderr, "Could not create JACK client.\n");
		exit(1);
	}
	jack_set_process_callback (jack_client, process, 0);
	output_port = jack_port_register(jack_client, outname, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	output_midimsg[0] = 0;
	output_midimsg[1] = 0;
	output_midimsg[2] = 0;
	jack_activate(jack_client);

	return 0;
}

//void freeSeq() {
//	if ( snd_seq_close(seq_handle) < 0 ) {
//		printf("Error closing socket!\n");
//	}
//}
