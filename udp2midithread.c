#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "midi2udpthread.h"

// Midi stuff
unsigned char udp2midi_midimsg[MIDI_MESSAGE_LENGTH];
snd_seq_t *udp2midi_seq_handle;
int midi_out_port;
snd_midi_event_t *udp2midi_eventparser;
snd_seq_event_t *udp2midi_midi_event;


int udp2midi_initSeq();
void udp2midi_freeSeq();

int udp2midi_init() {
	// Initialize midi port
	if (udp2midi_initSeq() == 1) {
		return 1;
	}
	
	return 0;
}

void * udp2midithread_run() {
	struct sockaddr_in serv_addr;
	int udp_socket;
		
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
			add_ip(from_ip);
		
		} else {

			// Send to MIDI
			printf("udp2midi: Sending event: 0x%x 0x%x 0x%x\n", udp2midi_midimsg[0], udp2midi_midimsg[1], udp2midi_midimsg[2]);
	
			if ( snd_midi_event_encode(udp2midi_eventparser, udp2midi_midimsg, MIDI_MESSAGE_LENGTH, udp2midi_midi_event) < 0) {
				printf("Error encoding midi event!\n");
			}
			
			snd_midi_event_reset_encode(udp2midi_eventparser);
			
			if (udp2midi_midi_event->type == SND_SEQ_EVENT_NOTEON) {
				printf("udp2midi: Note on: %d, channel %d\n", udp2midi_midi_event->data.note.note, udp2midi_midi_event->data.control.channel);
			} else if (udp2midi_midi_event->type == SND_SEQ_EVENT_NOTEOFF){
				printf("udp2midi: Note off: %d, channel %d\n", udp2midi_midi_event->data.note.note, udp2midi_midi_event->data.control.channel);
			}
				
			snd_seq_ev_set_subs(udp2midi_midi_event);
			snd_seq_ev_set_direct(udp2midi_midi_event);
			snd_seq_ev_set_source(udp2midi_midi_event, midi_out_port);
				
			snd_seq_event_output_direct(udp2midi_seq_handle, udp2midi_midi_event);
			
			snd_seq_free_event(udp2midi_midi_event);
		}
	}
}

int udp2midi_initSeq() {
	char portname[64] = "DSMIDIWIFI UDP2MIDI OUT";

	if (snd_seq_open(&udp2midi_seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
    	printf("udp2midi: Error opening ALSA sequencer.\n");
    	return 1;
  	}
	
	snd_seq_set_client_name(udp2midi_seq_handle, "DSMIDIWIFI UDP2MIDI");
	
	
	midi_out_port = snd_seq_create_simple_port(udp2midi_seq_handle, portname, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
	SND_SEQ_PORT_TYPE_APPLICATION);
	
	if (midi_out_port < 0) {
		printf("udp2midi: Error creating MIDI port!\n");
		
		snd_seq_close(udp2midi_seq_handle);
		return 1;
	}
	
	if (snd_midi_event_new(MIDI_MESSAGE_LENGTH, &udp2midi_eventparser) != 0) {
		printf("udp2midi: Error making midi event parser!\n");
		
		snd_seq_close(udp2midi_seq_handle);
		return 1;
	}
	snd_midi_event_init(udp2midi_eventparser);
	
	udp2midi_midi_event = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t));
	
	return 0;
}

void udp2midi_freeSeq() {
	if ( snd_seq_close(udp2midi_seq_handle) < 0 ) {
		printf("udp2midi: Error closing socket!\n");
	}
}
