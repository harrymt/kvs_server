#include "server.h"

#include <pthread.h>
#include "safe_functions.h"

/**
 * Entry point for the server.
 */
int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s data-port control-port\n", argv[0]);
        exit(-1);
    }

    // Setup control and data ports.
    int cport = atoi(argv[2]), dport = atoi(argv[1]);

    // Starts a server thread to listen on the data and control ports.
    struct tuple_ports *ports = malloc_safe(sizeof(struct tuple_ports));
    ports->dport = dport;
    ports->cport = cport;
    pthread_t main_thread;
    if (pthread_create(&main_thread, NULL, initiate_servers, ports) < 0) {
        perror_exit("Could not create initiate servers thread.");
    }

    if(pthread_join(main_thread, NULL) != 0) {
        perror_exit("Could not join main thread.");
    }

    return 0;
}
