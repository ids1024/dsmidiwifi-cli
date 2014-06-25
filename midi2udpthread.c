#include "midi2udpthread.h"

#include <arpa/inet.h>
#include <string.h>

// Midi stuff
unsigned char midi2udp_midimsg[MIDI_MESSAGE_LENGTH];
extern snd_seq_t *seq_handle;
int midi_in_port;
snd_midi_event_t *mid2udp_eventparser;
snd_seq_event_t *midi2udp_midi_event;

int npfd;
struct pollfd *pfd;
		
struct ds_ips {
	char* addr;
	struct ds_ips *next;
};

struct ds_ips *ds_ips_first;


int midi2udp_initSeq();	
void midi2udp_freeSeq();


int midi2udp_init() {
	// Initialize midi port
	if (midi2udp_initSeq() == 1) {
		return 1;
	}
	
	// start expecing MIDI events
	npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
	pfd = (struct pollfd *)malloc(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

	ds_ips_first = malloc(sizeof(struct ds_ips));
	ds_ips_first->addr = NULL;
	ds_ips_first->next = NULL;
	
	return 0;
}

void add_ip(char ip[]) {
	struct ds_ips *cur;

	printf("add_ip %s", ip);
	if (ds_ips_first->addr == NULL) {
		ds_ips_first->addr = ip;
		return;
	}
	for (cur = ds_ips_first; cur->next != NULL; cur=cur->next) {
		if (strcmp(cur->addr, ip) == 0) // Duplicates won't be added
			return;
	}
	cur->next = malloc(sizeof(struct ds_ips));
	ds_ips_first->addr = ip;
	ds_ips_first->next = NULL;
}

void * midi2udpthread_run() {
	int udp_socket;

	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	while(1) {
		if (poll(pfd, npfd, 250) > 0) {
			
			printf("midi2udp: got midi event!\n");
			
			// Get MIDI event
			snd_seq_event_input(seq_handle, &midi2udp_midi_event);
			
			if (snd_midi_event_decode(mid2udp_eventparser, midi2udp_midimsg, MIDI_MESSAGE_LENGTH, midi2udp_midi_event) < 0) {
				printf("midi2udp: Error decoding midi event!\n");
			} else {
				struct ds_ips *cur;

				// Send it over UDP
				for (cur = ds_ips_first; cur->next != NULL; cur=cur->next) {
					int to_;
					struct sockaddr_in to_addr;

					printf("%s\n", cur->addr);
					inet_pton(AF_INET, cur->addr, &to_);
					sendto(udp_socket, (char*)midi2udp_midimsg, MIDI_MESSAGE_LENGTH,0 , (struct sockaddr *)&to_addr, sizeof(to_addr));
				}
			}
			
			snd_seq_free_event(midi2udp_midi_event);
			
			snd_midi_event_reset_decode(mid2udp_eventparser);
		}
	}
}

int midi2udp_initSeq() {
	char portname[64] = "in";

	midi_in_port = snd_seq_create_simple_port(seq_handle, portname, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION);
	
	if (midi_in_port < 0) {
		printf("midi2udp: Error creating MIDI port!\n");
		
		snd_seq_close(seq_handle);
		return 1;
	}
	
	if (snd_midi_event_new(MIDI_MESSAGE_LENGTH, &mid2udp_eventparser) != 0) {
		printf("midi2udp: Error making midi event parser!\n");
		
		snd_seq_close(seq_handle);
		return 1;
	}
	snd_midi_event_init(mid2udp_eventparser);
	
	midi2udp_midi_event = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t));
	
	return 0;
}
