
#ifndef _test_helpers_h_
#define _test_helpers_h_

#define MAX_MSG_LENGTH 1024

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif


void leave_server();
void connect_to_server(int port);

void my_assert_equals(char* a, char* b, char* test_name);
void test_cmd(char* cmd, char* res, char* test_name);

#endif
