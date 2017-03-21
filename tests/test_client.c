#include "test_client.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../source/kv.h"
#include "../source/parser.h"
#include "../source/debug.h"
#include "../source/server.h"
#include "../source/socket-helper.h"
#include "../source/message_manager.h"

pthread_t control_thread;
pthread_t data_thread;

void my_assert_equals(char* a, char* b, char* test_name) {
	int test_success = strcmp(a, b);
	if(test_success) {
		char o[512];
		sprintf(o, "ASSERTION FAILED: %s: ('%s' != '%s')", test_name, a, b);
		perror_line(o);
	} else {
		printf("> Passed: '%s'\n", test_name);
	}
	fflush(stdout);
}


void start_test_server(int port, enum SERVER_TYPE type) {
	struct server_config *config = malloc(sizeof(struct server_config));
	config->port = port;
	config->type = type;

	if(type == CONTROL) {
		start_server(config, control_thread);
	} else if (type == DATA) {
		start_server(config, data_thread);
	}
}

void leave_server(int connection) {
	close(connection);
}

void stop_server(enum SERVER_TYPE type) {
	if(type == CONTROL) {
		pthread_join(control_thread, NULL);
	} else if(type == DATA) {
		pthread_join(data_thread, NULL);
	}
}


void send_cmd(char* input, int connection) {
	if(write(connection, input, strlen(input) + 1) < 0) perror_line("send"); // TODO change to write
}


int connect_to_server(int port) {
	int my_test_sock = socket(AF_INET, SOCK_STREAM, 0); if(my_test_sock == -1) perror_line("Error opening socket");

	struct in_addr server_addr;
	if(!inet_pton(AF_INET, "127.0.0.1", &server_addr)) perror_line("inet_aton");

	struct sockaddr_in connection;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_family = AF_INET;
	connection.sin_port = htons(port);

	int sock = connect(my_test_sock, (const struct sockaddr*) &connection, sizeof(connection));
	if (sock != 0) perror_line("Have you started the server?");

	char client_message[LINE];
	memset(client_message, 0, LINE);
	int read_size = read_message(my_test_sock, &client_message);
	client_message[read_size] = '\0';
	printf("Recevied MOTD.\n"); fflush(stdout);
	return my_test_sock;
}


void test_cmd(char* cmd, char* res, char* test_name, int connection) {
	char output[LINE] = {0};
	send_cmd(cmd, connection);
	read_message(connection, output);
	my_assert_equals(output, res, test_name);
//	sleep(1); // Sleep between test responses
}
