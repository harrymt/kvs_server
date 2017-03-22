/* Server program for key-value store. */
#include <sys/socket.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "server.h"


int main(int argc, char** argv) {
	// Buffer single lines
	setlinebuf(stdout);

	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(-1);
	}

	/* Setup control and data ports. */
	int cport = atoi(argv[2]), dport = atoi(argv[1]);

	struct tuple_ports *ports = malloc(sizeof(struct tuple_ports));
	ports->dport = dport;
	ports->cport = cport;
	pthread_t main_thread;
	if (pthread_create(&main_thread, NULL, initiate_servers, ports) < 0) {
		perror_line("Could not start server.");
		exit(-1);
	}

	pthread_join(main_thread, NULL);

	return 0;
}
