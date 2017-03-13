/* Server program for key-value store. */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include "kv.h"
#include "parser.h"
#include "server.h"
#include "debug.h"
#include "server-utils.h"


/* A worker thread. You should write the code of this function. */
void* worker(void* p) {
	return NULL;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(1);
	}

	/* Setup control and data ports. */
	int cport = atoi(argv[2]), dport = atoi(argv[1]);
	DEBUG_PRINT(("Correct args used cport:%d, dport:%d.\n", cport, dport));

	/* Create & Bind a socket, then Listen on the port. */
	int file_descriptor = setup_socket(dport);

	struct sockaddr_in peer_addr;
  socklen_t peer_addr_size;
  // char line[LINE];

	printf("Server started.\n");
	int running = true;
	while(running) {

		peer_addr_size = sizeof(struct sockaddr_in);

	 	int connection = accept(file_descriptor, (struct sockaddr *) &peer_addr, &peer_addr_size);
		if (connection == -1) {
			perror("Error accepting connection");
			exit(1);
		}

		DEBUG_PRINT(("Found a connection, fd/connection:%d backlog:%d.\n", connection, LISTEN_BACKLOG));

		// TODO, Read and Write??

		// we now have an open connection
		// handle it, possibly set run = 0
		// if we want to shut down.
	  // enum DATA_CMD cmd;
		// char* key;
		// char* text;
		// parse_d(buffer, &cmd, &key, &text);
	}

	return 0;
}




	// 	To close a socket, both the client and server must call close(fd) with the file descriptor for
	// this socket. Optionally, one can call shutdown before close which has the effect that any
	// further read/write operations (depending on the “how” parameter) will return end of file
	// resp. failure. Every socket that you open with socket(), you must close again with close().
	// Using shutdown() is optional but recommended.
	// int shutdown(int fd, int how);
	// // how: SHUT_RD, SHUT_WR, SHUT_RDWR
	// // 0 = success, (-1) = error
	// int close(int fd);
	// // 0 = success, (-1) = error

