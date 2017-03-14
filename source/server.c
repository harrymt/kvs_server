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
#include <unistd.h>
#include "kv.h"
#include "parser.h"
#include "server.h"
#include "debug.h"
#include "server-utils.h"

struct socket_info
{
    int port;
};


/* A worker thread. You should write the code of this function. */
void* worker(void* args) {
	/* Extract the thread arguments */
	struct socket_info *socket = (struct socket_info*) args;

	// TODO, Read and Write??

	// we now have an open connection
	// handle it, possibly set run = 0
	// if we want to shut down.
  // enum DATA_CMD cmd;
	// char* key;
	// char* text;

	return socket;
}


void *server_listen(void* args) {
	/* Extract the thread arguments */
	struct socket_info *socket = (struct socket_info*) args;

	/* Create & Bind a socket, then Listen on the port. */
	int server_socket = setup_socket(socket->port);

	struct sockaddr_in peer_addr;
    socklen_t peer_address;
  pthread_t worker_thread; // TODO do we need an array of these??

	printf("Server started.\n");
	int running = true;
	while(running) {

		peer_address = sizeof(struct sockaddr_in);
		int connection = accept(server_socket, (struct sockaddr *) &peer_addr, &peer_address);
		if (connection == -1) {
			perror("Error accepting connection");
			DEBUG_PRINT(("Could not accept a connection, just contining, backlog:%d.\n", LISTEN_BACKLOG));
			continue;
		}

		DEBUG_PRINT(("Found a connection, fd/connection:%d backlog:%d.\n", connection, LISTEN_BACKLOG));

 	  if (pthread_create(&worker_thread, NULL, worker, socket) != 0) {
        perror("Could not create a worker thread");
        // free(clientAddr);
        // free(wa);
        // close(clientSocket);
        // close(serverSocket);
        pthread_exit(NULL);
    }

		// TODO pass in some workerarguments a socket and if setup completed?

		// parse_d(buffer, &cmd, &key, &text);
	}

}

/**
 * Handle creating a new server thread with the port information.
 */
void start_server(struct socket_info *i, pthread_t t) {
	if (pthread_create(&t, NULL, server_listen, &i) < 0) {
    perror("Could not start server.");
    exit(-1);
  }

	DEBUG_PRINT(("OK: Successfully started server listening on port:%d.\n", i->port));
}


int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(-1);
	}

	/* Setup control and data ports. */
	int cport = atoi(argv[2]), dport = atoi(argv[1]);
	DEBUG_PRINT(("OK: Correct args used dport:%d, cport:%d.\n", dport, cport));

	pthread_t data_thread = pthread_self(), control_thread = pthread_self();

	struct socket_info *data_info = malloc(sizeof(struct socket_info));
	struct socket_info *control_info = malloc(sizeof(struct socket_info));
  data_info->port = dport;
  control_info->port = cport;

  start_server(data_info, data_thread);
  start_server(control_info, control_thread);

	pthread_join(data_thread, NULL);
	pthread_join(control_thread, NULL);

	// TODO destory any mutexs or condition variables

	return 0; // pthread_exit(NULL); Need this instead?
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

