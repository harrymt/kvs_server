
#ifndef _server_h_
#define _server_h_

#include "debug.h"
#include <pthread.h>

#define NTHREADS 4
#define LISTEN_BACKLOG 10
#define MAX_MESSAGE_SIZE 2000
#define MAX_QUEUE_SIZE 10

enum SERVER_TYPE { CONTROL, DATA };

struct tuple_ports {
	int cport;
	int dport;
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
    pthread_t *thread;
};

void* worker(void* args);
void *server_listen(void* args);
void *initiate_servers(void* args);
void init_worker_pool();
void init_pre_server_setup();
int poll_for_connections(int sock);

#endif
