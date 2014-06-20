#include <QCoreApplication>

#include "midi2udpthread.h"
#include "udp2midithread.h"


int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	Midi2UdpThread *midi2udpthread;
	Udp2MidiThread *udp2midithread;

	midi2udpthread = 0;
	udp2midithread = 0;
	
	midi2udpthread = new Midi2UdpThread();
	bool res = midi2udpthread->go();
	if(res == true) {
		printf("midi2udp initialized\n");
	} else {
		fprintf(stderr, "Error initializing midi2udp!");
		delete midi2udpthread;
		midi2udpthread = 0;
	}
	
	udp2midithread = new Udp2MidiThread();
	res = udp2midithread->go(midi2udpthread);
	if(res == true) {
		printf("udp2midi initialized\n");
	} else {
		fprintf(stderr, "Error initializing udp2midi!");
		delete udp2midithread;
		udp2midithread = 0;
	}

	return app.exec();
}
