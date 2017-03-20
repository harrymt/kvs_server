
#include <pthread.h>
#include "queue.h"

#ifndef _server_h_
#define _server_h_

#define NTHREADS 4
#define LISTEN_BACKLOG 10

#define perro(x) {fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno));exit(1);}

enum SERVER_TYPE { CONTROL, DATA };


struct queue_item
{
	int port;
	int sock;
	int type;
};

struct server_config
{
    int port;
    int s; // Socket
    int worker_num;
    enum SERVER_TYPE type;
};


struct worker_configuration
{
    int worker_number;
    int is_available;
    pthread_t *thread;

    int port;
    int sock; // Socket
    int worker_num;
    enum SERVER_TYPE type;
};

void* worker(void* args);
void start_server(struct server_config *i, pthread_t t);

int initiate_server(int cport, int dport);

#endif
