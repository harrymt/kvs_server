
#ifndef _server_h_
#define _server_h_

#define NTHREADS 4
#define LISTEN_BACKLOG 10

void* worker(void* p);

#endif
