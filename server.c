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
	p = NULL;
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

	printf("Server started.\n");
	while(1) { /* Hang TODO: Implement this*/ }

    return 0;
}

