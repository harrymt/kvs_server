/* Server program for key-value store. */
#include "server.h"

#include <sys/socket.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>

#include "server_helpers.h"
#include "protocol_manager.h"
#include "message_manager.h"
#include "queue.h"
#include "socket_helper.h"
#include "safe_functions.h"

pthread_mutex_t mutex_kill = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_kill = PTHREAD_COND_INITIALIZER;
int server_port_that_wants_to_die = 0;

int DATA_SOCKET;
int CONTROL_SOCKET;

// The queue for producer and consumers
Queue *worker_queue;
pthread_t worker_threads[NTHREADS];
struct worker_configuration *worker_thread_pool;

/**
 * Like the main function, creates 2 threads to listen to the ports give.
 */
void* initiate_servers(void* args) {
	/* Extract the server config arguments */
	struct tuple_ports *ports = (struct tuple_ports*) args;

	pthread_t data_thread = pthread_self(), control_thread = pthread_self();
	struct server_config *data_info = malloc_safe(sizeof(struct server_config));
	struct server_config *control_info = malloc_safe(sizeof(struct server_config));
	data_info->port = ports->dport;
	data_info->type = DATA;
	control_info->port = ports->cport;
	control_info->type = CONTROL;

	init_worker_pool();

	int number_of_servers_alive = 2;
	start_server(data_info, data_thread);
	start_server(control_info, control_thread);
	printf("Server started.\n");

	DEBUG_PRINT(("Debug mode activated."));

	pthread_mutex_lock_safe(&mutex_kill);
	while(server_port_that_wants_to_die == 0) {
		pthread_cond_wait_safe(&cond_kill, &mutex_kill); // wait on a condition variable

		if(server_port_that_wants_to_die == ports->cport) {
			number_of_servers_alive = 0;
			server_port_that_wants_to_die = 0;
		} else {
			DEBUG_PRINT(("BAD: Oh dear, trying to kill server that we don't have %d, ignore it.\n", server_port_that_wants_to_die));
			perror_exit("Trying to kill server on port that we don't have.");
		}
		pthread_mutex_unlock_safe(&mutex_kill);

		if(number_of_servers_alive == 0) {
			printf("Shutting down.\n");

			// Close sockets of both ports
			close_safe(DATA_SOCKET);
			close_safe(CONTROL_SOCKET);

			DEBUG_PRINT(("OK: All servers are dead (%d), stopping main thread.\n", number_of_servers_alive));
			break;
		}
	}

	return 0;
}

/**
 * Consumer, takes new connections from the Queue and handles that connection.
 */
void* worker(void* args) {
	int worker_number = *(int*)args; // Extract thread arguments

	if(pthread_detach(pthread_self()) != 0) {
		perror_exit("Error detaching thread.");
	}

	DEBUG_PRINT(("Worker %d created, waiting for new tasks...\n", worker_number));
	bool running = true;
	while(running) {

		queue_item current_queue_connection = queue_pop(worker_queue);

		char initial_message[512];
		build_initial_message(current_queue_connection.type, initial_message);

		int msg_error = send_message(current_queue_connection.sock, &initial_message);
		if(msg_error) { perror_exit("Error sending message"); }

		char client_message[MAX_MESSAGE_SIZE];

		while(true)
		{
			memset(client_message, 0, 256);

			int is_error = poll_for_connections(current_queue_connection.sock);
			if(is_error == -1) {
				DEBUG_PRINT(("Polling error %d.\n", is_error));
				continue;
			}
			int read_size = read_message(current_queue_connection.sock, &client_message);

			if(read_size == 0) {
				DEBUG_PRINT(("%d client disconnected.\n", 1));
				break;
			}

			int is_success = run_command(current_queue_connection.type, &client_message, read_size);

			// Send message back to the client
			msg_error = send_message(current_queue_connection.sock, &client_message);
			if(msg_error) { perror_exit("Error sending message"); }

			if(is_success == R_DEATH) { // They want to die
				close_safe(current_queue_connection.sock);
				break;

			} else if(is_success == R_SHUTDOWN) {
				pthread_mutex_lock_safe(&mutex_kill);
				server_port_that_wants_to_die = current_queue_connection.port;
				pthread_cond_signal_safe(&cond_kill);
				pthread_mutex_unlock_safe(&mutex_kill);

				running = false;
				break;
			}
		}
	}

    return 0;
}

/**
 * Producer, listens for new connections then adds them to a worker queue.
 */
void *server_listen(void* args) {

	/* Extract the server config arguments */
	struct server_config *settings = (struct server_config*) args;

	/* Create & Bind a socket, then Listen on the port. */
	int server_socket = setup_socket(settings->port);
	if(settings->type == DATA) {
		DATA_SOCKET = server_socket;
	} else if(settings->type == CONTROL) {
		CONTROL_SOCKET = server_socket;
	}

	struct sockaddr_in peer_addr;
    socklen_t address_size;

	int running = true;
	while(running) {
		// Poll for new connection
		int is_error = poll_for_connections(server_socket);
		if(is_error == -1) {
			DEBUG_PRINT(("Polling error %d.\n", is_error));
			continue;
		}

		// Connect to the client
		address_size = sizeof(struct sockaddr_in);
		int connection = accept_connection(server_socket, &peer_addr, address_size);  // accept_connection(server_socket, &peer_addr, address_size);
		if (connection == -1) {
			DEBUG_PRINT(("Could not accept a connection, just containing, backlog:%d.\n", LISTEN_BACKLOG));
			continue;
		}

		// Create queue item & add client to the consumer queue
		queue_item i = {.sock = connection, .port = settings->port, .type = settings->type };
		queue_push(worker_queue, i);
	}
	return 0;
}

/**
 * Creates the Queue for the producer/consumers, and creates the worker threads.
 */
void init_worker_pool() {
    // Setup a queue for workers to consume
	worker_queue = make_queue(MAX_QUEUE_SIZE);

    // Setup a number of workers
	worker_thread_pool = malloc_safe(sizeof(struct worker_configuration) * NTHREADS);

	for(int w = 0; w < NTHREADS; w++) {
		if (pthread_create(&worker_threads[w], NULL, worker, &w) < 0) {
			perror_exit("Could not create a worker thread");
		}
		DEBUG_PRINT(("Creating new thread %d\n", w));

		struct worker_configuration newThread;
		newThread.worker_number = w;
		newThread.thread = &worker_threads[w];
    	worker_thread_pool[w] = newThread;
    }
}
