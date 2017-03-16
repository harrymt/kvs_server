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
#include "../source/kv.h"
#include "../source/parser.h"
#include "../source/debug.h"
#include "../source/server.h"
#include "../source/socket-helper.h"

#define MAX_MSG_LENGTH 1024

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#define perro(x) {fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno));exit(1);}

int my_test_sock;

static void send_cmd(char* input) {
	if(send(my_test_sock, input, strlen(input)+1, 0) < 0) perro("send");
}

static void receive_one_line(char* buf) {
	int filled = 0;
	filled = recv(my_test_sock, buf, MAX_MSG_LENGTH-1, 0);
	buf[filled] = '\0';
}

void connect_to_server(int port) {
	my_test_sock = socket(AF_INET, SOCK_STREAM, 0); if(my_test_sock == -1) perro("Error opening socket");

	struct in_addr server_addr;
	if(!inet_aton("127.0.0.1", &server_addr)) perro("inet_aton");

	struct sockaddr_in connection; connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(port);

	if (connect(my_test_sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perro("Have you started the server?");

	char output[MAX_MSG_LENGTH] = {0};
	receive_one_line(output); // Receive Motd
}

void my_assert_equals(char* a, char* b, char* test_name) {
	int test_success = strcmp(a, b);
	if(test_success) {
		char o[512];
		sprintf(o, "%s: (%s != %s)", test_name, a, b);
		perro(o)
	} else {
		printf("%s: Passed, (%s == %s).\n", test_name, a, b);
	}
}

void test_cmd(char* cmd, char* res, char* test_name) {
	char output[MAX_MSG_LENGTH] = {0};
	char input[MAX_MSG_LENGTH] = {0};
	send_cmd(cmd);
	receive_one_line(output);
	my_assert_equals(output, res, test_name);
	sleep(1); // Sleep between test responses
}


int main(int argc, char** argv) {
	int dport = atoi(argv[1]);
	int cport = atoi(argv[2]);

	// Connect to Data server with a port
	connect_to_server(dport);

	// Run tests
	my_assert_equals("0", "0", "Testing my assert equals function.");
	test_cmd("COUNT", "0\n", "Count test.");
	test_cmd("COUNT", "0\n", "Count test twice.");

	test_cmd("PUT name Harry", "Success.\n", "Put test.");
	test_cmd("EXISTS name", "1\n", "Exists test.");
	test_cmd("EXISTS invalidkey", "0\n", "Doesn't exist.");

	close(my_test_sock);

	// Connect to Control server with a port
	connect_to_server(cport);

	// Run tests
	my_assert_equals("0", "0", "Testing my assert equals function.");
	test_cmd("COUNT", "0\n", "Count test.");
	test_cmd("COUNT", "0\n", "Count test twice.");
	test_cmd("SHUTDOWN", "Shutting down.\n", "Test successful shutdown.");

	printf("==== SUCCESS ====\nAll tests pass\n");
	return 0;
}
