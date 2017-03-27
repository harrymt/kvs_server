#include "message_manager.h"

#include <unistd.h>
#include "parser.h"
#include "server.h"

/**
 * Builds the first message to send to the client.
 * This is based on if its a CONTROL server or DATA server.
 */
void build_initial_message(int type, void* message) {
    int result = 0;
    if(type == CONTROL) {
        result = sprintf(message, "%s", "Welcome to the server.\n");
    } else if(type == DATA) {
        result = sprintf(message, "%s", "Welcome to the KV store.\n");
    }
    if(result < 0) {
        perror_exit("sprintf failed.")
    }
}

/**
 * Reads a line from the client (socket), placing the result into the message variable.
 */
int read_message(int socket, void* message) {
    int is_error = read(socket, message, LINE);

    if(is_error < 0) {
        printf("%d, %d\n", is_error, errno);
        perror_exit("Read message failed!");
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
        perror_exit("Unexpected error in send_message()!");
    }

    return 0; // Success
}
