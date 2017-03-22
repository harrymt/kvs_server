
#ifndef _server_helpers_h_
#define _server_helpers_h_

#include <pthread.h>
#include <sys/socket.h>
#include "server.h"

void start_server(struct server_config *i, pthread_t t);
int poll_for_connections(int sock);
int accept_connection(int sock, void *address, socklen_t size);

#endif
