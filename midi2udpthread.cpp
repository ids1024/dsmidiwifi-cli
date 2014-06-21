#include "midi2udpthread.h"

#include <arpa/inet.h>

// Midi stuff
unsigned char midi2udp_midimsg[MIDI_MESSAGE_LENGTH];
snd_seq_t *midi2udp_seq_handle;
int midi_in_port;
snd_midi_event_t *mid2udp_eventparser;
snd_seq_event_t *midi2udp_midi_event;

int npfd;
struct pollfd *pfd;
		
set<string> ds_ips;


bool midi2udp_initSeq();	
void midi2udp_freeSeq();


bool midi2udp_init()
{
	// Initialize midi port
	bool res = midi2udp_initSeq();
	if(res == false) {
		return false;
	}
	
	// start expecing MIDI events
	npfd = snd_seq_poll_descriptors_count(midi2udp_seq_handle, POLLIN);
	pfd = (struct pollfd *)malloc(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(midi2udp_seq_handle, pfd, npfd, POLLIN);
	
	return true;
}

void add_ip(string ip)
{
	ds_ips.insert(ip); // Duplicates won't be added
}

void * midi2udpthread_run(void *)
{
	QUdpSocket *udpSocket;
	udpSocket = new QUdpSocket(0);
	
	forever {
		if (poll(pfd, npfd, 250) > 0) {
			
			printf("midi2udp: got midi event!\n");
			
			// Get MIDI event
			snd_seq_event_input(midi2udp_seq_handle, &midi2udp_midi_event);
			
			int res = snd_midi_event_decode(mid2udp_eventparser, midi2udp_midimsg, MIDI_MESSAGE_LENGTH, midi2udp_midi_event);
			
			if( res < 0 ) {
				printf("midi2udp: Error decoding midi event!\n");
			} else {
				// Send it over UDP
				for(set<string>::iterator ip_it = ds_ips.begin(); ip_it != ds_ips.end(); ++ip_it)
				{
					QString to_((*ip_it).c_str());
					QHostAddress to(to_);
					udpSocket->writeDatagram((char*)midi2udp_midimsg, MIDI_MESSAGE_LENGTH, to, DS_PORT);
				}
			}
			
			snd_seq_free_event(midi2udp_midi_event);
			
			snd_midi_event_reset_decode(mid2udp_eventparser);
		}
	}
}

bool midi2udp_initSeq()
{
	if(snd_seq_open(&midi2udp_seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
    	printf("midi2udp: Error opening ALSA sequencer.\n");
    	return false;
  	}
	
	snd_seq_set_client_name(midi2udp_seq_handle, "DSMIDIWIFI MIDI2UDP");
	
	char portname[64] = "DSMIDIWIFI MIDI2UDP IN";
	
	int res = midi_in_port = snd_seq_create_simple_port(midi2udp_seq_handle, portname, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
              SND_SEQ_PORT_TYPE_APPLICATION);
	
	if(res < 0) {
		printf("midi2udp: Error creating MIDI port!\n");
		
		snd_seq_close(midi2udp_seq_handle);
		return false;
	}
	
	res = snd_midi_event_new(MIDI_MESSAGE_LENGTH, &mid2udp_eventparser);
	if(res != 0) {
		printf("midi2udp: Error making midi event parser!\n");
		
		snd_seq_close(midi2udp_seq_handle);
		return false;
	}
	snd_midi_event_init(mid2udp_eventparser);
	
	midi2udp_midi_event = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t));
	
	return true;
}

void midi2udp_freeSeq()
{
	int res = snd_seq_close(midi2udp_seq_handle);
	if( res < 0 ) {
		printf("midi2udp: Error closing socket!\n");
	}
}
