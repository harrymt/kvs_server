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
#include "socket-helper.h"

#define MAX_MESSAGE_SIZE 2000

pthread_mutex_t mutex_kill =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_kill = PTHREAD_COND_INITIALIZER;
int server_port_that_wants_to_die = 0;

/**
 *
 */
int read_message(struct socket_info *data, char* message) {
	printf("read_message start.\n");
	// Receiving a client message and place it into the message array
	int is_error = recv(data->s, message, strlen(message), 0);

	if(is_error <= 0 && (message[is_error - 1] != '\n')) {
		printf("is_error %d.\n", is_error);
		message[is_error - 1] = '\0';
	} else {
		perror("Read Message failed!");
		free(data);
		pthread_exit(NULL);
	}

	printf("read_message end, is_error %d.\n", is_error);
	return is_error;
}

/**
 * Send message to the connected client.
 * TODO add params
 */
void send_message(struct socket_info *data, void* message) {
    int is_error = send(data->s, message, strlen(message), 0);

    printf("send_message start, is_error %d.\n", is_error);
    if (is_error == -1 && (errno == ECONNRESET || errno == EPIPE))
    {
    	printf("BAD A %d.\n", is_error);
        fprintf(stderr, "Socket %d disconnected.\n", data->s);
        close(data->s);
        free(data);
        pthread_exit(NULL);
    }
    else if (is_error == -1)
    {
    	printf("BAD B %d.\n", is_error);
        perror("Unexpected error in send_message()!");
        free(data);
        pthread_exit(NULL);
    }
    printf("send_message end, is_error %d.\n", is_error);
}


/* A worker thread. You should write the code of this function. */
void* worker(void* args) {
	/* Extract the thread arguments */
	struct socket_info *data = (struct socket_info*) args;

	printf("Worker 1 executing task.\n");

    /* This tells the pthreads library that no other thread is going to
       join() this thread. This means that, once this thread terminates,
       its resources can be safely freed (instead of keeping them around
       so they can be collected by another thread join()-ing this thread) */
    pthread_detach(pthread_self());

    // Setup the initial message
    char initial_message[100];
    sprintf(initial_message, "Welcome to the server.\nYou are being served by worker %d\n> ", data->worker_num);

    send_message(data, &initial_message);
    char client_message[MAX_MESSAGE_SIZE];
    while(1)
    {

    	int read_size = read_message(data, client_message);

    	// Put a C style string terminator at the end of the message
    	// client_message[read_size] = '\0';

    	printf("Message received '%d'.\n", read_size);
    	break; // TODO remove me to see the bug
    	// TODO, do something with the message
    	send_message(data, &client_message); // TODO remove

		// Clear the message buffer, ready to accept more messages
		//memset(client_message, 0, MAX_MESSAGE_SIZE);
    }

    pthread_exit(NULL);

	// TODO, Read and Write??

	// we now have an open connection
	// handle it, possibly set run = 0
	// if we want to shut down.
	// enum DATA_CMD cmd;
	// char* key;
	// char* text;


	// 	To close a socket, both the client and server must call close(fd) with the file descriptor for
	// this socket. Optionally, one can call shutdown before close which has the effect that any
	// further read/write operations (depending on the �how� parameter) will return end of file
	// resp. failure. Every socket that you open with socket(), you must close again with close().
	// Using shutdown() is optional but recommended.
	// int shutdown(int fd, int how);
	// // how: SHUT_RD, SHUT_WR, SHUT_RDWR
	// // 0 = success, (-1) = error
	// int close(int fd);
	// // 0 = success, (-1) = error


	//	pthread_mutex_lock(&mutex_kill);
	//	server_port_that_wants_to_die = our_socket->port;
	//	pthread_cond_signal(&cond_kill);
	//	pthread_mutex_unlock(&mutex_kill);
}


void *server_listen(void* args) {
	/* Extract the thread arguments */
	struct socket_info *our_socket = (struct socket_info*) args;

	/* Create & Bind a socket, then Listen on the port. */
	int server_socket = setup_socket(our_socket->port);

	struct sockaddr_in peer_addr;
    socklen_t peer_address;
    pthread_t worker_thread; // TODO do we need an array of these??

	int running = true;
	while(running) {
		peer_address = sizeof(struct sockaddr_in);
		int connection = accept(server_socket, (struct sockaddr *) &peer_addr, &peer_address);

		if (connection == -1) {
			perror("Error accepting connection");
			DEBUG_PRINT(("Could not accept a connection, just containing, backlog:%d.\n", LISTEN_BACKLOG));
			continue;
		}

		printf("Got a connection.\n");
		our_socket->s = connection;
		our_socket->worker_num = 1;
		if (pthread_create(&worker_thread, NULL, worker, our_socket) != 0) {
			perror("Could not create a worker thread");
			pthread_exit(NULL);
		}

		printf("Delegating to worker 1.\n");
	}
	return 0;
}

/**
 * Handle creating a new server thread with the port information.
 */
void start_server(struct socket_info *i, pthread_t t) {
	if (pthread_create(&t, NULL, server_listen, i) < 0) {
		perror("Could not start server.");
		exit(-1);
	}

	DEBUG_PRINT(("OK: Successfully started server listening on port:%d.\n", ((struct socket_info *)i)->port));
}

int main(int argc, char** argv) {
	// Buffer single lines
	setlinebuf(stdout);

	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(-1);
	}

	/* Setup control and data ports. */
	int cport = atoi(argv[2]), dport = atoi(argv[1]);

	pthread_t data_thread = pthread_self(), control_thread = pthread_self();

	struct socket_info *data_info = malloc(sizeof(struct socket_info));
	struct socket_info *control_info = malloc(sizeof(struct socket_info));
	data_info->port = dport;
	control_info->port = cport;

	int number_of_servers_alive = 2;
	start_server(data_info, data_thread);
	start_server(control_info, control_thread);

	printf("Server started.\n");

	pthread_mutex_lock(&mutex_kill);
	while(server_port_that_wants_to_die == 0) {
		pthread_cond_wait(&cond_kill, &mutex_kill); // wait on a condition variable

		if(server_port_that_wants_to_die == cport) {
			DEBUG_PRINT(("OK: Killing control server port:%d.\n", cport));
			pthread_join(control_thread, NULL);
			number_of_servers_alive--;

			DEBUG_PRINT(("OK: Killing data server port:%d.\n", dport));
			pthread_join(data_thread, NULL);
			number_of_servers_alive--;

			server_port_that_wants_to_die = 0;
		} else {
			DEBUG_PRINT(("BAD: Oh dear, trying to kill server that we don't have %d, ignore it.\n", server_port_that_wants_to_die));
			exit(1); // TODO remove me
		}
		pthread_mutex_unlock(&mutex_kill);

		if(number_of_servers_alive == 0) {
			printf("Shutting down.\n");
			DEBUG_PRINT(("OK: All servers are dead, stopping main thread num:%d.\n", number_of_servers_alive));
			break;
		}
	}

	// TODO destroy any Mutexs or condition variables

	return 0;
}


