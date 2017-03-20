

#ifndef _message_manager_h_
#define _message_manager_h_


void get_initial_message(int type, int worker, void* message);

/**
 *
 */
int read_message(int socket, void* message);

/**
 * Send message to the connected client.
 * TODO add params
 */
int send_message(int socket, void* message);

#endif
