#include <sys/socket.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "kv.h"
#include "parser.h"
#include "server.h"
#include "debug.h"
#include "protocol_manager.h"
#include "socket-helper.h"
#include "message_manager.h"


/**
 * Handle creating a new server thread with the port information.
 */
void start_server(struct server_config *i, pthread_t t) {
	if (pthread_create(&t, NULL, server_listen, i) < 0) {
		perror_line("Could not start server.");
		exit(-1);
	}

	DEBUG_PRINT(("OK: Successfully started %d server listening on port:%d.\n", ((struct server_config *)i)->type, ((struct server_config *)i)->port));
}
