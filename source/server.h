
#include <pthread.h>

#ifndef _server_h_
#define _server_h_

#define NTHREADS 4
#define LISTEN_BACKLOG 10

#define perro(x) {fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno));exit(1);}

enum SERVER_TYPE { CONTROL, DATA };


struct socket_info
{
    int port;
    int s; // Socket
    int worker_num;
    enum SERVER_TYPE type;
};

void* worker(void* args);

#endif
