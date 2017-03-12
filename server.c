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

#define NTHREADS 4
#define LISTEN_BACKLOG 10

/*
 * Debug printf statements.
 * Taken from here: http://stackoverflow.com/a/1941337
 * Called like
 *  DEBUG_PRINT(("Hi %d", 1));
 * Note: The extra parentheses are necessary, because some older C compilers don't support var-args in macros.
 */
/* 1: enables extra print statements, 0: disable */
#define DEBUG 1
#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


/* A worker thread. You should write the code of this function. */
void* worker(void* p) {
	return NULL;
}

/**
 * Creates a socket for the server, if successful we return a file
 * descriptor, an int referring to a file data struct, (managed by the OS).
 * Close a socket by calling close(file_descriptor), then shutdown(file_descriptor, SHUT_RDWR)
 *
 * Returns: file descriptor, otherwise -1 on error.
 */
int create_socket() {
	int domain = AF_INET; /* AF_INET for TCP/IP, AF_UNIX for unix sockets */
	int type = SOCK_STREAM;
	int protocol = 0; /* Any protocol */
	return socket(domain, type, protocol);
}

/**
 * Assigns an address to the created scoket.
 *
 * param fd: File descriptor of the created socket.
 * param port: Port for the socket.
 *
 * Returns: 0 on success, -1 on error.
 */
int bind_socket(int fd, int port) {
	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	memset(&sa, 0, len);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY); // or INADDR_LOOPBACK
	sa.sin_port = htons(port);
	return bind(fd, (struct sockaddr *) &sa, len);
}


int main(int argc, char** argv) {
  int cport, dport; /* control and data ports. */
	int err_temp; /* Temp error number */

	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(1);
	} else {
		cport = atoi(argv[2]);
		dport = atoi(argv[1]);
	}

	DEBUG_PRINT(("Correct args used cport:%d, dport:%d.\n", cport, dport));

	// Create a Socket
	int file_descriptor = create_socket();
	if(file_descriptor == -1) {
		err_temp = errno; // Save error number
		printf("Error creating socket %s\n", strerror(err_temp));
		exit(1);
	}

	DEBUG_PRINT(("Successfully created socket, fd:%d.\n", file_descriptor));

	int is_error = bind_socket(file_descriptor, dport); // TODO, could be dport?
	if(is_error == -1) {
		err_temp = errno; // Save error number
		printf("Error binding socket %s\n", strerror(err_temp));
		exit(1);
	}

	DEBUG_PRINT(("Successfully binded socket, fd:%d.\n", file_descriptor));

	struct sockaddr_in peer_addr;
  socklen_t peer_addr_size;
  // char line[LINE];

	printf("Server started.\n");
	int running = true;
	while(running) {

		int is_error = listen(file_descriptor, LISTEN_BACKLOG);
		if(is_error == -1) {
			err_temp = errno; // Save error number
			printf("Error listening %s\n", strerror(err_temp));
			exit(1);
		}

		DEBUG_PRINT(("Listening successfully, backlog:%d.\n", LISTEN_BACKLOG));

		peer_addr_size = sizeof(struct sockaddr_in);
		// TODO, should we be casting (struct sockaddr *) ????
	 	int connection = accept(file_descriptor, (struct sockaddr *) &peer_addr, &peer_addr_size);
		if (connection == -1) {
			err_temp = errno; // Save error number
			printf("Error accepting connection %s\n", strerror(err_temp));
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

