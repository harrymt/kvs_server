#include "test_client.h"

#include <unistd.h>

void test_data_protocol(int dport);
void test_control_protocol(int cport);

/**
 * Start of tests. Starts a sever and runs lots of different
 * commands on it, testing the output.
 */
int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(-1);
	}

	/* Setup control and data ports. */
	int cport = atoi(argv[2]), dport = atoi(argv[1]);
	printf("Warning, if you have used ports (c: %d, d: %d) recently, they may not be available yet\n", cport, dport);

	my_assert_equals("0", "0", "Testing my assert equals function.");

	// Start a test server
	struct tuple_ports *ports = malloc(sizeof(struct tuple_ports));
	ports->dport = dport;
	ports->cport = cport;
	pthread_t main_thread = pthread_self();
	if (pthread_create(&main_thread, NULL, initiate_servers, ports) < 0) {
		perror_exit("Could not start server.");
		exit(-1);
	}
	printf("Waiting for server to start, please wait...\n"); fflush(stdout);
	sleep(1); // Wait for server to start

	test_data_protocol(dport);
	test_control_protocol(cport);

	fflush(stdout);
	pthread_join(main_thread, NULL);
	printf("==== SUCCESS ====\nAll tests pass\n");
	return 0;
}

/**
 * Client worker to fire off a single request, then leave the server.
 */
void* test_multi_connections(void* args) {
	int port = *(int *) args;
	int fd = connect_to_server(port);
	test_cmd("COUNT\n", "2\n", "Count 4+ multi connections.", fd);
	test_cmd("\n", "Goodbye.\n", "Leave test.", fd);

	return NULL;
}

/**
 * Client worker to fire off 10 requests, then leave the server.
 */
void* test_multi_connections_lots(void* args) {
	int port = *(int *) args;
	int fd = connect_to_server(port);
	for(int i = 0; i < 10; i++) {
		test_cmd("COUNT\n", "2\n", "Count 4+ multi connections lots.", fd);
	}
	test_cmd("\n", "Goodbye.\n", "Leave test.", fd);

	return NULL;
}

/**
 * Test the Data protocol.
 */
void test_data_protocol(int dport) {
	printf("Testing data server...\n"); fflush(stdout);

	// Connect to Data server with a port
	int datac = connect_to_server(dport);


	// Run tests on data
	test_cmd("COUNT\n", "0\n", "Count test.", datac);
	test_cmd("COUNT\n", "0\n", "Count test twice.", datac);

	test_cmd("PUT name Harry\n", "Success.\n", "Put test.", datac);
	test_cmd("PUT name Harry\n", "Error storing key.\n", "Put test already exists.", datac);
	test_cmd("EXISTS name\n", "1\n", "Exists test.", datac);
	test_cmd("EXISTS invalidkey\n", "0\n", "Doesn't exist.", datac);

	test_cmd("GET name\n", "Harry\n", "Get value from key.", datac);
	test_cmd("GET name\n", "Harry\n", "Get multiple.", datac);

	test_cmd("DELETE name\n", "Success.\n", "Delete key that exists.", datac);
	test_cmd("DELETE name\n", "Error, no key found.\n", "Delete key that doesn't exists.", datac);


	// Test invalid commands
	test_cmd("COUNT a b c\n", "Error, command too long.\n", "Invalid command, too many parameters.", datac);
	test_cmd("PUT\n", "Error, command too short.\n", "Invalid command, not enough parameters.", datac);
	test_cmd("ABC\n", "Error, command not found.\n", "Invalid command, not found.", datac);
	test_cmd("COUNT", "Error, can't find EOL, line too long.\n", "Invalid command, no EOL.", datac);


	// Fill up the KVS
	for(int j = 0; j < 99; j++) {
		char msg[100];
		sprintf(msg, "PUT unique_%d TheValue", j);
		test_cmd(msg, "Success.\n", "Test under 100 values in kvs.", datac);
	}

	// Try to add more than the maximum (100 - 1)
	test_cmd("PUT errorKey xxx", "Error storing key.\n", "Test over 100 values in kvs.", datac);
	test_cmd("COUNT\n", "99\n", "Count full kvs.", datac);

	// Delete those keys again
	for(int j = 0; j < 99; j++) {
		char msg[100];
		sprintf(msg, "DELETE unique_%d", j);
		test_cmd(msg, "Success.\n", "Just delete these keys.", datac);
	}

	test_cmd("PUT name Harry\n", "Success.\n", "Put same after delete.", datac);
	test_cmd("GET name\n", "Harry\n", "Get value after re-add.", datac);

	test_cmd("\n", "Goodbye.\n", "Leave test.", datac);
	leave_server(datac);

	// Run multi connection tests
	int datac2 = connect_to_server(dport);
	test_cmd("COUNT\n", "1\n", "Count multi connections test.", datac2);

	int second_datac = connect_to_server(dport);
	test_cmd("COUNT\n", "1\n", "Count multi connections test A.", datac2);
	test_cmd("COUNT\n", "1\n", "Count multi connections test B.", second_datac);

	test_cmd("PUT fruit Apple\n", "Success.\n", "Count multi connections PUT.", datac2);
	test_cmd("GET fruit\n", "Apple\n", "Count multi connections GET.", second_datac);
	test_cmd("\n", "Goodbye.\n", "Leave test again.", second_datac);
	test_cmd("\n", "Goodbye.\n", "Leave test multi again.", datac2);

	// Test 4+ connections
	int number = NTHREADS + 10;
	pthread_t threads[number];
	for(int i = 0; i < number; i++) {
		if (pthread_create(&(threads[i]), NULL, test_multi_connections, &dport) < 0) {
			perror_exit("Could not start server.");
			exit(-1);
		}
	}

	// Test 4+ connections
	int number2 = NTHREADS + 10;
	pthread_t threads2[number2];
	for(int i = 0; i < number2; i++) {
		if (pthread_create(&(threads2[i]), NULL, test_multi_connections_lots, &dport) < 0) {
			perror_exit("Could not start server.");
			exit(-1);
		}
	}
}

void test_control_protocol(int cport) {
	printf("Testing control server...\n"); fflush(stdout);

	// Connect to Control server with a port
	int controlc = connect_to_server(cport);

	// Run tests on control
	test_cmd("COUNT", "2\n", "Count test on control retains value.", controlc);
	test_cmd("\n", "Goodbye.\n", "Leave control test.", controlc);
	leave_server(controlc);

	// Test leave server
	controlc = connect_to_server(cport);
	test_cmd("SHUTDOWN", "Shutting down.\n", "Test successful shutdown.", controlc);
	leave_server(controlc);
}




