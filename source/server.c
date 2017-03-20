/* Server program for key-value store. */
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
#include "queue.h"

#define MAX_MESSAGE_SIZE 2000

pthread_mutex_t mutex_kill = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_kill = PTHREAD_COND_INITIALIZER;
int server_port_that_wants_to_die = 0;

pthread_t worker_threads[NTHREADS];
int worker_ids[NTHREADS];
struct worker_configuration *worker_thread_pool;

int queue_index = -1;
#define MAX_QUEUE_SIZE 10
struct queue_item *queue;


pthread_cond_t Buffer_Not_Full=PTHREAD_COND_INITIALIZER;
pthread_cond_t Buffer_Not_Empty=PTHREAD_COND_INITIALIZER;
pthread_mutex_t consuming=PTHREAD_MUTEX_INITIALIZER;


// TODO get total number of workers working, maybe just have a semaphore???

/**
 *
 */
int read_message(int socket, void* message) {
	int is_error = read(socket, message, 255);

	if(is_error < 0) {
		perro("Read Message failed!");
		pthread_exit(NULL);
	}

	return is_error;
}

/**
 * Send message to the connected client.
 * TODO add params
 */
bool send_message(int socket, void* message) {
    int is_error = write(socket, message, strlen(message));

    if (is_error == -1 && (errno == ECONNRESET || errno == EPIPE))
    {
    	// TODO there is an error here... when 1 client disconnects, others cant reconnect
    	fprintf(stderr, "Socket %d disconnected.\n", socket);
        return false;
    }
    else if (is_error == -1)
    {
    	perro("Unexpected error in send_message()!");
        pthread_exit(NULL);
        return false;
    }

    return true;
}


void get_initial_message(int type, int worker, void* message) {
	char* data_message = "Welcome to the KV store.\n"; // \n
	char* control_message = "Welcome to the server.\n"; // \n
	if(type == CONTROL) {
		if(DEBUG) {
			sprintf(message, "%s (worker %d).\n", control_message, worker);
		} else {
			sprintf(message, "%s", control_message);
		}
	} else if(type == DATA) {
		if(DEBUG) {
			sprintf(message, "%s(worker %d).\n", data_message, worker);
		} else {
			sprintf(message, "%s", data_message);
		}
	}
}

void make_worker_available(int worker_number) {
	if(worker_number > NTHREADS) { perror("WARNING worker number doesn't exist, too high!"); }
	worker_thread_pool[worker_number].is_available = 0;
}




void* worker(void* args) {
	/* Extract the thread arguments */
	int worker_number = *(int*)args;

	printf("Worker %d created, waiting for new tasks...\n", worker_number);
	bool running = true;
	while(running) {
		pthread_mutex_lock(&consuming);
		while(queue_index < 0) // While there is nothing in the queue
		{
			pthread_cond_wait(&Buffer_Not_Empty, &consuming);
		}
		printf("Consuming queue index : %d \n", queue_index);
		int current_queue_item = queue_index;
		struct queue_item current_queue_connection = queue[current_queue_item];
		queue_index--; // pop item off queue
		pthread_mutex_unlock(&consuming);
		pthread_cond_signal(&Buffer_Not_Full);

		printf("Worker %d executing queue item %d.\n", worker_number, current_queue_item);
		printf("current_queue_connection: sock %d, type %d, port %d\n", current_queue_connection.sock, current_queue_connection.type, current_queue_connection.port);

		/* This tells the pthreads library that no other thread is going to
		   join() this thread. This means that, once this thread terminates,
		   its resources can be safely freed (instead of keeping them around
		   so they can be collected by another thread join()-ing this thread) */
	   // pthread_detach(pthread_self());

		char initial_message[512];
		get_initial_message(current_queue_connection.type, worker_number, initial_message);

		bool still_connected = send_message(current_queue_connection.sock, &initial_message);
		if(!still_connected) {
			// TODO this could be a function
			fprintf(stderr, "Send message failure, worker %d.\n", worker_number);
			return 0; // TODO change
		}

		char client_message[MAX_MESSAGE_SIZE];

		while(true)
		{
			memset(client_message, 0, 256);

			int read_size = read_message(current_queue_connection.sock, &client_message);

			// DEBUG_PRINT(("Data port(%d), socket(%d), type(%d), worker(%d).\n", data->port, data->s, data->type, data->worker_num));

			int is_success = run_command(current_queue_connection.type, &client_message);

			// Send message back to the client
			bool is_successful = send_message(current_queue_connection.sock, &client_message);
			if(!is_successful) {
				// TODO this could be a function
				fprintf(stderr, "Send message failure, worker %d.\n", worker_number);
				break;
			}

			if(is_success == R_DEATH) { // They want to die
				close(current_queue_connection.sock);
				break;
			} else if(is_success == R_SHUTDOWN) {
				printf("Shutting down.\n");
				pthread_mutex_lock(&mutex_kill);
				server_port_that_wants_to_die = current_queue_connection.port;
				pthread_cond_signal(&cond_kill);
				pthread_mutex_unlock(&mutex_kill);
			}
		}

//		make_worker_available(worker_number);  // Make this worker available again
	}

    return 0;
}


int find_available_worker_thread() {
	for(int i = 0; i < NTHREADS; i++) {
		if(worker_thread_pool[i].is_available == 0) {
			// Available
			worker_thread_pool[i].is_available = 1; // Set unavailable
			return i;
		}
	}
	return -1; // Error
}


void init_worker_threads() {
    for(int w = 0; w < NTHREADS; w++) {
    	struct worker_configuration newThread;
    	newThread.worker_number = w;
    	newThread.is_available = 0; // Set available
    	worker_ids[w] = w;

    	printf("Creating new thread %d\n", worker_ids[w]);

		if (pthread_create(&worker_threads[w], NULL, worker, &worker_ids[w]) != 0) {
			perro("Could not create a worker thread");
			break;
		}

		newThread.thread = &worker_threads[w];
    	worker_thread_pool[w] = newThread;
    }
}

void shutdown_worker_thread_pool() {
	for(int w = 0; w < NTHREADS; w++) {
		pthread_join(worker_threads[w], NULL);
	}
}

void queue_add(int type, int sock, int port) {
	queue_index++;

	// Add item to queue
	struct queue_item i;
	i.sock = sock;
	i.port = port;
	i.type = type;
	queue[queue_index] = i;
}


void *server_listen(void* args) {

	/* Extract the server config arguments */
	struct server_config *settings = (struct server_config*) args;

	/* Create & Bind a socket, then Listen on the port. */
	int server_socket = setup_socket(settings->port);

	struct sockaddr_in peer_addr;
    socklen_t peer_address;

    // Setup a queue for workers to consume
    queue_index = -1;
    queue = malloc(sizeof(struct queue_item) * MAX_QUEUE_SIZE);

    // Setup a number of workers
    worker_thread_pool = malloc(sizeof(struct worker_configuration) * NTHREADS);

    // Start all the workers for data threads
	init_worker_threads();

	int running = true;
	while(running) {
		// Wait for connection
		peer_address = sizeof(struct sockaddr_in);
		int connection = accept(server_socket, (struct sockaddr *) &peer_addr, &peer_address);

		if (connection == -1) {
			perro("Error accepting connection");
			DEBUG_PRINT(("Could not accept a connection, just containing, backlog:%d.\n", LISTEN_BACKLOG));
			continue;
		}

		printf("Got a connection.\n");
		pthread_mutex_lock(&consuming);
		if(queue_index == MAX_QUEUE_SIZE)
		{
			pthread_cond_wait(&Buffer_Not_Full, &consuming);
		}
		printf("Produce : %d \n", queue_index);
		// Add to a queue for workers to consume.
		queue_add(settings->type, connection, settings->port);
		pthread_mutex_unlock(&consuming);

		// Signal consumers to consume the queue
		pthread_cond_signal(&Buffer_Not_Empty);

	}
	return 0;
}


/**
 * Handle creating a new server thread with the port information.
 */
void start_server(struct server_config *i, pthread_t t) {
	if (pthread_create(&t, NULL, server_listen, i) < 0) {
		perro("Could not start server.");
		exit(-1);
	}

	DEBUG_PRINT(("OK: Successfully started server listening on port:%d.\n", ((struct server_config *)i)->port));
}

int initiate_server(int cport, int dport) {
	pthread_t data_thread = pthread_self(), control_thread = pthread_self();

	struct server_config *data_info = malloc(sizeof(struct server_config));
	struct server_config *control_info = malloc(sizeof(struct server_config));
	data_info->port = dport;
	data_info->type = DATA;
	control_info->port = cport;
	control_info->type = CONTROL;

	int number_of_servers_alive = 2;
	start_server(data_info, data_thread);
//	start_server(control_info, control_thread);

	printf("Server started.\n");

	pthread_mutex_lock(&mutex_kill);
	while(server_port_that_wants_to_die == 0) {
		pthread_cond_wait(&cond_kill, &mutex_kill); // wait on a condition variable

		if(server_port_that_wants_to_die == cport) {
			DEBUG_PRINT(("OK: Killing control server port:%d.\n", cport));
			pthread_join(control_thread, NULL);
			number_of_servers_alive--;

			DEBUG_PRINT(("OK: Killing data server port:%d.\n", dport));
			pthread_join(data_thread, NULL);
			number_of_servers_alive--;

			server_port_that_wants_to_die = 0;
		} else {
			DEBUG_PRINT(("BAD: Oh dear, trying to kill server that we don't have %d, ignore it.\n", server_port_that_wants_to_die));
			exit(1); // TODO remove me
		}
		pthread_mutex_unlock(&mutex_kill);

		if(number_of_servers_alive == 0) {
			printf("Shutting down.\n");
			DEBUG_PRINT(("OK: All servers are dead, stopping main thread num:%d.\n", number_of_servers_alive));
			break;
		}
	}

	// TODO destroy any Mutexs or condition variables

	return 0;
}







// TODO lock worker pool?
//		int worker_number = find_available_worker_thread();
//		while((worker_number = find_available_worker_thread()) == -1) {
//			// BLOCK until some workers are available
//		}
//		our_socket->worker_num = worker_number;
//		printf("Delegating to worker %d.\n", worker_number);
//		if (pthread_create(&worker_threads[worker_number], NULL, worker, our_socket) != 0) {
//			perro("Could not create a worker thread");
//			break;
//		}
// TODO unlock worker pool?


//void setup_data_thread_pool(pthread_t* threads) {
//	pthread_t markerT[100];
//
//	/* Create S student threads */
//	for (i = 0; i < parameters.S; i++) {
//	  if(pthread_create(&studentT[i], NULL, student, &studentID[i])) {
//		fprintf(stderr, "Error creating student thread, student %d\n", i);
//		exit(1);
//	  }
//	}
//
//}


