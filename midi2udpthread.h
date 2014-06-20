#ifndef _MIDI2UDPTHREAD_
#define _MIDI2UDPTHREAD_

#include <QThread>
#include <QMutex>
#include <QtNetwork>

#include <set>
#include <string>

#include <alsa/asoundlib.h>

#include "settings.h"

using namespace std;

class Midi2UdpThread: public QThread
{
	Q_OBJECT
	
	public:
		Midi2UdpThread(QObject *parent = 0);
		~Midi2UdpThread();
	
		bool go();
		
		void add_ip(string ip);
		
	protected:
		void run();
	
	private:
		bool initSeq();	
		void freeSeq();
		
		// Thread stuff
		QMutex mutex;
		bool abort;
	
		// Midi stuff
		unsigned char midimsg[MIDI_MESSAGE_LENGTH];
		snd_seq_t *seq_handle;
		int midi_in_port;
		snd_midi_event_t *eventparser;
		snd_seq_event_t *midi_event;
		
		int npfd;
		struct pollfd *pfd;
		
		set<string> ds_ips;
};

#endif
