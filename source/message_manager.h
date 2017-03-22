
#ifndef _message_manager_h_
#define _message_manager_h_

void build_initial_message(int type, int worker, void* message);
int read_message(int socket, void* message);
int send_message(int socket, void* message);

#endif
