#include "server_helpers.h"

#include <pthread.h>
#include <stdbool.h>
#include <poll.h>
#include <sys/socket.h>
#include "server.h"

/**
 * Handle creating a new server thread with the port information.
 */
void start_server(struct server_config *i, pthread_t t) {
	if (pthread_create(&t, NULL, server_listen, i) < 0) {
		perror_line("Could not start server.");
	}

	DEBUG_PRINT(("Successfully started %d server listening on port %d.\n", ((struct server_config *)i)->type, ((struct server_config *)i)->port));
}


/**
 * Polls for new connections
 */
int poll_for_connections(int sock) {
	struct pollfd pfd = {.fd = sock, .events = POLLIN };

	int POLL_TIMEOUT = 10000;
	int result;

	while(true) {
		DEBUG_PRINT(("Polling for %d...\n", POLL_TIMEOUT));

		result = poll(&pfd, 1, POLL_TIMEOUT);

		if ((pfd.revents & POLLNVAL) || (result == -1)) {
			return -1;

		} else if (pfd.revents & (POLLHUP | POLLERR)) {
			continue; // Client disconnected

		} else if (pfd.revents & POLLIN) {
			return 1;
		}
	}
	return -1; // client disconnected
}

int accept_connection(int sock, void *address, socklen_t size) {
	return accept(sock, (struct sockaddr *) address, &size);
}
