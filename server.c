/* Server program for key-value store. */

#include <stdlib.h>
#include <stdio.h>
#include "kv.h"
#include "parser.h"

#define NTHREADS = 4;
#define BACKLOG = 10;

/* Add anything you want here. */

/* A worker thread. You should write the code of this function. */
void* worker(void* p) {
	return NULL;
}

/* You may add code to the main() function. */
int main(int argc, char** argv) {
   int cport, dport; /* control and data ports. */

	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(1);
	} else {
		cport = atoi(argv[2]);
		dport = atoi(argv[1]);
	}
	int running = 1;
	printf("Server started.\n");
	printf("Control Port [%d], Data Port [%d].\n", cport, dport);

	while(running) {
	  char line[LINE];

	  // enum DATA_CMD cmd;
		// char* key;
		// char* text;
		// parse_d(buffer, &cmd, &key, &text);
	}

	return 0;
}

