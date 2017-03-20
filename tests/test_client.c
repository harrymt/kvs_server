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

pthread_t control_thread;
pthread_t data_thread;

void my_assert_equals(char* a, char* b, char* test_name) {
	int test_success = strcmp(a, b);
	if(test_success) {
		char o[512];
		sprintf(o, "ASSERTION FAILED: %s: ('%s' != '%s')", test_name, a, b);
		perro(o);
	} else {
		printf("> Passed: '%s'\n", test_name);
	}
	fflush(stdout);
}


void start_test_server(int port, enum SERVER_TYPE type) {
	struct server_config *data_info = malloc(sizeof(struct server_config));
	data_info->port = port;
	data_info->type = type;
	if(type == CONTROL) {
		start_server(data_info, control_thread);
	} else if (type == DATA) {
		start_server(data_info, data_thread);
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
	if(write(connection, input, strlen(input) + 1) < 0) perro("send"); // TODO change to write
}

void receive_one_line(char* buf, int connection) {
	int filled = read(connection, buf, MAX_MSG_LENGTH-1); // TODO change to read
	buf[filled] = '\0';
}

int connect_to_server(int port) {
	int my_test_sock = socket(AF_INET, SOCK_STREAM, 0); if(my_test_sock == -1) perro("Error opening socket");

	struct in_addr server_addr;
	if(!inet_pton(AF_INET, "127.0.0.1", &server_addr)) perro("inet_aton");

	struct sockaddr_in connection;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_family = AF_INET;
	connection.sin_port = htons(port);

	if (connect(my_test_sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perro("Have you started the server?");

	char output[MAX_MSG_LENGTH] = {0};
	receive_one_line(output, my_test_sock); // Receive Motd
	return my_test_sock;
}


void test_cmd(char* cmd, char* res, char* test_name, int connection) {
	char output[MAX_MSG_LENGTH] = {0};
	send_cmd(cmd, connection);
	receive_one_line(output, connection);
	my_assert_equals(output, res, test_name);
//	sleep(1); // Sleep between test responses
}
