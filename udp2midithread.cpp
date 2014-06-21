#include <iostream>

#include "midi2udpthread.h"

// Midi stuff
unsigned char udp2midi_midimsg[MIDI_MESSAGE_LENGTH];
snd_seq_t *udp2midi_seq_handle;
int midi_out_port;
snd_midi_event_t *udp2midi_eventparser;
snd_seq_event_t *udp2midi_midi_event;


bool udp2midi_initSeq();
void udp2midi_freeSeq();


//Udp2MidiThread::~Udp2MidiThread()
//{
//	mutex.lock();
//	abort = true;
//	mutex.unlock();
//	wait();
//	freeSeq();
//}

bool udp2midi_init()
{
	// Initialize midi port
	bool res = udp2midi_initSeq();
	if(res == false) {
		return false;
	}
	
	// run thread
	//if(!isRunning()) {
	//	start(LowPriority);
	//}
	
	return true;
}

void * udp2midithread_run(void *)
{
	QUdpSocket *udpSocket = new QUdpSocket();
	bool bres = udpSocket->bind(PC_PORT);
	if( bres == false ) {
		char msg[256];
		sprintf(msg, "Could not bind to port %d!\n", PC_PORT);
		printf(msg);
//		return 1;
	}
	
	forever {
		
//		if (abort) {
//			delete udpSocket;
			
//			return;
//		}
		
		if( udpSocket->waitForReadyRead(250) == true ) {
			
			// Receive from UDP
			if( udpSocket->pendingDatagramSize() > MIDI_MESSAGE_LENGTH) {
				printf("udp2midi: received a message of %d bytes, but max length is %d byte\n", (int)udpSocket->pendingDatagramSize(), MIDI_MESSAGE_LENGTH);
			}
			
			QHostAddress from_address;
			int res = udpSocket->readDatagram((char*)udp2midi_midimsg, MIDI_MESSAGE_LENGTH, &from_address);
		
			if( res == -1 ) {
				printf("udp2midi: Error receiving data!\n");
			}
			
			if( (udp2midi_midimsg[0] == 0) && (udp2midi_midimsg[1] == 0) && (udp2midi_midimsg[2] == 0) ) {

				string from_ip = from_address.toString().toStdString();
			
				printf("Keepalive from: %s\n", from_ip.c_str());

				add_ip(from_ip);
			
			} else {

				// Send to MIDI
				printf("udp2midi: Sending event: 0x%x 0x%x 0x%x\n", udp2midi_midimsg[0], udp2midi_midimsg[1], udp2midi_midimsg[2]);
		
				res = snd_midi_event_encode(udp2midi_eventparser, udp2midi_midimsg, MIDI_MESSAGE_LENGTH, udp2midi_midi_event);
				
				if( res < 0) {
					printf("Error encoding midi event!\n");
				}
				
				snd_midi_event_reset_encode(udp2midi_eventparser);
				
				if(udp2midi_midi_event->type == SND_SEQ_EVENT_NOTEON) {
					printf("udp2midi: Note on: %d, channel %d\n", udp2midi_midi_event->data.note.note, udp2midi_midi_event->data.control.channel);
				} else if(udp2midi_midi_event->type == SND_SEQ_EVENT_NOTEOFF){
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
}

bool udp2midi_initSeq()
{
	if(snd_seq_open(&udp2midi_seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
    	printf("udp2midi: Error opening ALSA sequencer.\n");
    	return false;
  	}
	
	snd_seq_set_client_name(udp2midi_seq_handle, "DSMIDIWIFI UDP2MIDI");
	
	char portname[64] = "DSMIDIWIFI UDP2MIDI OUT";
	
	int res = midi_out_port = snd_seq_create_simple_port(udp2midi_seq_handle, portname, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
              SND_SEQ_PORT_TYPE_APPLICATION);
	
	if(res < 0) {
		printf("udp2midi: Error creating MIDI port!\n");
		
		snd_seq_close(udp2midi_seq_handle);
		return false;
	}
	
	res = snd_midi_event_new(MIDI_MESSAGE_LENGTH, &udp2midi_eventparser);
	if(res != 0) {
		printf("udp2midi: Error making midi event parser!\n");
		
		snd_seq_close(udp2midi_seq_handle);
		return false;
	}
	snd_midi_event_init(udp2midi_eventparser);
	
	udp2midi_midi_event = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t));
	
	return true;
}

void udp2midi_freeSeq()
{
	int res = snd_seq_close(udp2midi_seq_handle);
	if( res < 0 ) {
		printf("udp2midi: Error closing socket!\n");
	}
}
