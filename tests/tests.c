
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
#include "test_helpers.h"
#include "../source/kv.h"
#include "../source/parser.h"
#include "../source/debug.h"
#include "../source/server.h"
#include "../source/socket-helper.h"


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

	leave_server();


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
