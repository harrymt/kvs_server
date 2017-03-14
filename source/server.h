
#include <pthread.h>

#ifndef _server_h_
#define _server_h_

#define NTHREADS 4
#define LISTEN_BACKLOG 10

struct socket_info
{
    int port;
};

void* worker(void* args);

#endif
