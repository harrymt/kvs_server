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
#include "sync_handler.h"

#define MAX_MESSAGE_SIZE 2000
pthread_mutex_t mutex_kill = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_kill = PTHREAD_COND_INITIALIZER;
int server_port_that_wants_to_die = 0;


// TODO get total number of workers working, maybe just have a semaphore???

/**
 *
 */
int read_message(struct socket_info *data, void* message) {
	int is_error = read(data->s, message, 255);

	if(is_error < 0) {
		perro("Read Message failed!");
		free(data);
		pthread_exit(NULL);
	}

	return is_error;
}

/**
 * Send message to the connected client.
 * TODO add params
 */
bool send_message(struct socket_info *data, void* message) {
    int is_error = write(data->s, message, strlen(message));

    if (is_error == -1 && (errno == ECONNRESET || errno == EPIPE))
    {
    	// TODO there is an error here... when 1 client disconnects, others cant reconnect
    	fprintf(stderr, "Socket %d disconnected.\n", data->s);
        return false;
    }
    else if (is_error == -1)
    {
    	perro("Unexpected error in send_message()!");
        free(data);
        pthread_exit(NULL);
        return false;
    }

    return true;
}


void get_initial_message(int type, int worker, void* message) {
	char* data_message = "Welcome to the KV store.\n"; // \n
	char* control_message = "Welcome to the server.\n"; // \n
	if(type == CONTROL) {
		if(DEBUG) {
			sprintf(message, "%s (worker %d).\n", control_message, worker);
		} else {
			sprintf(message, "%s", control_message);
		}
	} else if(type == DATA) {
		if(DEBUG) {
			sprintf(message, "%s(worker %d).\n", data_message, worker);
		} else {
			sprintf(message, "%s", data_message);
		}
	}
}


/* A worker thread. You should write the code of this function. */
void* worker(void* args) {
	/* Extract the thread arguments */
	struct socket_info *data = (struct socket_info*) args;

	printf("Worker %d executing task.\n", data->worker_num);

    /* This tells the pthreads library that no other thread is going to
       join() this thread. This means that, once this thread terminates,
       its resources can be safely freed (instead of keeping them around
       so they can be collected by another thread join()-ing this thread) */
    pthread_detach(pthread_self());

    char initial_message[512];
    get_initial_message(data->type, data->worker_num, initial_message);

    bool still_connected = send_message(data, &initial_message);
    if(!still_connected) {
    	// TODO this could be a function
    	fprintf(stderr, "Send message failure, worker %d.\n", data->worker_num);
    	return 0; // TODO change
    }
    char client_message[MAX_MESSAGE_SIZE];

    while(true)
    {
    	memset(client_message, 0, 256);

    	// TODO: new issue, if the client disconnects we get weird looping again
    	int read_size = read_message(data, &client_message);

    	// DEBUG_PRINT(("Data port(%d), socket(%d), type(%d), worker(%d).\n", data->port, data->s, data->type, data->worker_num));

    	int is_success = run_command(data, &client_message);
    	if(is_success == R_DEATH) { // They want to die
    		close(data->s);
    		break;
    	} else if(is_success == R_SHUTDOWN) {
    		printf("Shutting down.\n");
    		pthread_mutex_lock(&mutex_kill);
			server_port_that_wants_to_die = data->port;
			pthread_cond_signal(&cond_kill);
			pthread_mutex_unlock(&mutex_kill);
    	}

    	// Print client return message
    	DEBUG_PRINT((">%s", client_message));

    	bool is_successful = send_message(data, &client_message);
        if(!is_successful) {
        	// TODO this could be a function
        	fprintf(stderr, "Send message failure, worker %d.\n", data->worker_num);
        	// TODO we dont want to just call pthread here, we should call a different exit thing
        	return 0;
        }
    }

    return 0;
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
			perro("Error accepting connection");
			DEBUG_PRINT(("Could not accept a connection, just containing, backlog:%d.\n", LISTEN_BACKLOG));
			continue;
		}

		printf("Got a connection.\n");
		our_socket->s = connection;
		our_socket->worker_num = 1;
		if (pthread_create(&worker_thread, NULL, worker, our_socket) != 0) {
			perro("Could not create a worker thread");
			break;
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
		perro("Could not start server.");
		exit(-1);
	}

	DEBUG_PRINT(("OK: Successfully started server listening on port:%d.\n", ((struct socket_info *)i)->port));
}

int initiate_server(int cport, int dport) {
	pthread_t data_thread = pthread_self(), control_thread = pthread_self();

	struct socket_info *data_info = malloc(sizeof(struct socket_info));
	struct socket_info *control_info = malloc(sizeof(struct socket_info));
	data_info->port = dport;
	data_info->type = DATA;
	control_info->port = cport;
	control_info->type = CONTROL;

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



// we now have an open connection
// handle it, possibly set run = 0
// if we want to shut down.
// enum DATA_CMD cmd;
// char* key;
// char* text;


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


//	pthread_mutex_lock(&mutex_kill);
//	server_port_that_wants_to_die = our_socket->port;
//	pthread_cond_signal(&cond_kill);
//	pthread_mutex_unlock(&mutex_kill);
