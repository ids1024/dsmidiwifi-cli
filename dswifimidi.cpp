#include <pthread.h>

#include <QCoreApplication>

#include "midi2udpthread.h"
#include "udp2midithread.h"


int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	pthread_t midi2udpthread;
	pthread_t udp2midithread;

	bool res = midi2udp_init();
	pthread_create(&midi2udpthread, NULL, midi2udpthread_run, NULL);
	if(res == true) {
		printf("midi2udp initialized\n");
	} else {
		fprintf(stderr, "Error initializing midi2udp!");
		exit(1);
	}
	
	res = udp2midi_init();
	if(res == true) {
		printf("udp2midi initialized\n");
	pthread_create(&udp2midithread, NULL, udp2midithread_run, NULL);
	} else {
		fprintf(stderr, "Error initializing udp2midi!");
		exit(1);
	}

	return app.exec();
}
