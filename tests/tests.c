
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
#include "../source/protocol_manager.h"
#include "../source/queue.h"
#include "../source/socket-helper.h"
#include "test_client.h"

void* test_multi_connections(void* args) {
	int port = *(int *) args;
	int fd = connect_to_server(port);
	test_cmd("COUNT\n", "2\n", "Count 4+ multi connections.", fd);
	test_cmd("\n", "", "Leave test.", fd);

	return NULL;
}

void* test_multi_connections_lots(void* args) {
	int port = *(int *) args;
	int fd = connect_to_server(port);
	for(int i = 0; i < 10; i++) {
		test_cmd("COUNT\n", "2\n", "Count 4+ multi connections lots.", fd);
//		sleep(1);
	}
	test_cmd("\n", "", "Leave test.", fd);

	return NULL;
}

void test_data_server(int dport) {
	printf("Testing data server...\n"); fflush(stdout);

	// Connect to Data server with a port
	int datac = connect_to_server(dport);

	printf("Connected to data server %d.\n", datac); fflush(stdout);

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

	test_cmd("PUT name Harry\n", "Success.\n", "Put same after delete.", datac);
	test_cmd("GET name\n", "Harry\n", "Get value after re-add.", datac);

	test_cmd("\n", "", "Leave test.", datac);
	leave_server(datac);

	// Run multi connection tests
	int datac2 = connect_to_server(dport);
	test_cmd("COUNT\n", "1\n", "Count multi connections test.", datac2);

	int second_datac = connect_to_server(dport);
	test_cmd("COUNT\n", "1\n", "Count multi connections test A.", datac2);
	test_cmd("COUNT\n", "1\n", "Count multi connections test B.", second_datac);

	test_cmd("PUT fruit Apple\n", "Success.\n", "Count multi connections PUT.", datac2);
	test_cmd("GET fruit\n", "Apple\n", "Count multi connections GET.", second_datac);
	test_cmd("\n", "", "Leave test again.", second_datac);
	test_cmd("\n", "", "Leave test multi again.", datac2);

	// Test 4+ connections
	int number = NTHREADS + 10;
	pthread_t threads[number];
	for(int i = 0; i < number; i++) {
		if (pthread_create(&(threads[i]), NULL, test_multi_connections, &dport) < 0) {
			perror_line("Could not start server.");
			exit(-1);
		}
	}

	// Test 4+ connections
	int number2 = NTHREADS + 10;
	pthread_t threads2[number2];
	for(int i = 0; i < number2; i++) {
		if (pthread_create(&(threads2[i]), NULL, test_multi_connections_lots, &dport) < 0) {
			perror_line("Could not start server.");
			exit(-1);
		}
	}
}

void test_control_server(int cport) {


	// Connect to Control server with a port // TODO enable
	int controlc = connect_to_server(cport);

	// Run tests on control
	test_cmd("COUNT", "2\n", "Count test on control retains value.", controlc);
	test_cmd("\n", "", "Leave control test.", controlc);
	leave_server(controlc);

	// Test leave server
	controlc = connect_to_server(cport);
	test_cmd("SHUTDOWN", "Shutting down.\n", "Test successful shutdown.", controlc);
	leave_server(controlc);

}

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: %s data-port control-port\n", argv[0]);
		exit(-1);
	}


	/* Setup control and data ports. */
	int cport = atoi(argv[2]), dport = atoi(argv[1]);
	printf("==== WARNING ====\nIf you have used these ports recently c: %d, d: %d, they may not be available until the OS port available timeout is finished\n==== WARNING ====\n", cport, dport);

	my_assert_equals("0", "0", "Testing my assert equals function.");

	// Start a test server
	struct tuple_ports *ports = malloc(sizeof(struct tuple_ports));
	ports->dport = dport;
	ports->cport = cport;
	pthread_t main_thread;
	if (pthread_create(&main_thread, NULL, initiate_servers, ports) < 0) {
		perror_line("Could not start server.");
		exit(-1);
	}

	test_data_server(dport);
	test_control_server(cport);

	pthread_join(main_thread, NULL);
	printf("==== SUCCESS ====\nAll tests pass\n");
	return 0;
}


