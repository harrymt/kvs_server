#include "message_manager.h"

#include <unistd.h>
#include "parser.h"
#include "server.h"


/**
 * Builds the first message to send to the client.
 * This is based on if its a CONTROL server or DATA server.
 */
void build_initial_message(int type, int worker, void* message) {
	char* data_message = "Welcome to the KV store.\n";
	char* control_message = "Welcome to the server.\n";
	if(type == CONTROL) {
	#ifdef DEBUG
			sprintf(message, "%s (worker %d).\n", control_message, worker);
	#else
			sprintf(message, "%s", control_message);
	#endif

	} else if(type == DATA) {
	#ifdef DEBUG
			sprintf(message, "%s (worker %d).\n", data_message, worker);
	#else
			sprintf(message, "%s", data_message);
	#endif
	}
}

/**
 * Reads a line from the client (socket), placing the result into the message variable.
 */
int read_message(int socket, void* message) {
	int is_error = read(socket, message, LINE);

	if(is_error < 0) {
		perror_line("Read message failed!");
	}

	return is_error;
}

/**
 * Sends message to the connected client (socket).
 */
int send_message(int socket, void* message) {
    int is_error = write(socket, message, strlen(message));

    if (is_error == -1 && (errno == ECONNRESET || errno == EPIPE)) {
    	DEBUG_PRINT(("Socket %d disconnected.\n", socket));
        return -1;

    } else if (is_error == -1) {
    	perror_line("Unexpected error in send_message()!");
    }

    return 0; // Success
}
