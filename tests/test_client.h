
#ifndef _test_helpers_h_
#define _test_helpers_h_

#include "../source/server.h"

void leave_server(int connection);
int connect_to_server(int port);
void my_assert_equals(char* a, char* b, char* test_name);
void test_cmd(char* cmd, char* res, char* test_name, int connection);
void start_test_server(int port, enum SERVER_TYPE type);

#endif
