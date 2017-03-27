
#ifndef _server_utils_h_
#define _server_utils_h_

/**
 * Creates a socket, binds it to the port then returns
 * the file descriptor.
 */
int setup_socket(int port);
int build_socket();

#endif
