#ifndef _UDP2MIDITHREAD_
#define _UDP2MIDITHREAD_

#include <QThread>
#include <QMutex>
#include <QtNetwork>

#include <alsa/asoundlib.h>

#include "settings.h"

#include "midi2udpthread.h"

class Udp2MidiThread: public QThread
{
	Q_OBJECT
	
	public:
		Udp2MidiThread(QObject *parent = 0);
		~Udp2MidiThread();
		
		bool go(Midi2UdpThread *midi2udp);
		
	protected:
        void run();
		
	private:
		bool initSeq();
		void freeSeq();
		
		Midi2UdpThread *midi2udp;
		
		// Thread stuff
		QMutex mutex;
		bool abort;
		
		// Midi stuff
		unsigned char midimsg[MIDI_MESSAGE_LENGTH];
		snd_seq_t *seq_handle;
		int midi_out_port;
		snd_midi_event_t *eventparser;
		snd_seq_event_t *midi_event;
		
};

#endif
