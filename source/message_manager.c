#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "debug.h"
#include "parser.h"
#include "server.h"
#include "message_manager.h"


void get_initial_message(int type, int worker, void* message) {
	char* data_message = "Welcome to the KV store.\n";
	char* control_message = "Welcome to the server.\n";
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

/**
 * Reads a line from the socket, placing the result into the message variable.
 */
int read_message(int socket, void* message) {
	int is_error = read(socket, message, LINE);

	if(is_error < 0) {
		perro("Read message failed!");
	}

	return is_error;
}

/**
 * Send message to the connected client.
 * TODO add params
 */
int send_message(int socket, void* message) {
    int is_error = write(socket, message, strlen(message));

    if (is_error == -1 && (errno == ECONNRESET || errno == EPIPE))
    {
    	// TODO there is an error here... when 1 client disconnects, others cant reconnect
    	fprintf(stderr, "Socket %d disconnected.\n", socket);
        return 1;
    }
    else if (is_error == -1)
    {
    	perro("Unexpected error in send_message()!");
        pthread_exit(NULL);
        return 1;
    }

    return 0;
}
