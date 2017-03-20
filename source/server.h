


#ifndef _server_h_
#define _server_h_

#include <pthread.h>
#include <semaphore.h>

#define NTHREADS 4
#define LISTEN_BACKLOG 10
#define MAX_MESSAGE_SIZE 2000
#define MAX_QUEUE_SIZE 10

#define perro(x) {fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno));exit(1);}

enum SERVER_TYPE { CONTROL, DATA };

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
void start_server(struct server_config *i, pthread_t t);
void *server_listen(void* args);
int initiate_server(int cport, int dport);
void init_worker_pool();

#endif
