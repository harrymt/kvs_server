#include "socket-helper.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include "kv.h"
#include "parser.h"
#include "debug.h"
#include "protocol_manager.h"
#include "server.h"


/**
 * Assigns an address to the created socket.
 *
 * param fd: File descriptor of the created socket.
 * param port: Port for the socket.
 *
 * Returns: 0 on success, -1 on error.
 */
int bind_socket(int fd, int port) {
  struct sockaddr_in sa;
  socklen_t len = sizeof(sa); // TODO maybe have to keep this memory somewhere
  memset(&sa, 0, len);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY); // or INADDR_LOOPBACK
  sa.sin_port = htons(port);
  return bind(fd, (struct sockaddr *) &sa, len);
}

/**
 * Creates a socket for the server, if successful we return a file
 * descriptor, an int referring to a file data struct, (managed by the OS).
 * Close a socket by calling close(file_descriptor), then shutdown(file_descriptor, SHUT_RDWR)
 *
 * Returns: file descriptor, otherwise -1 on error.
 */
int build_socket() {
  int domain = AF_INET; /* IPv4 TCP */
  int type = SOCK_STREAM; // TCP/IP
  int protocol = 0; /* Any protocol */
  // DEBUG_PRINT(("Attempting to build %d socket.\n", type));
  return socket(domain, type, protocol);
}

/**
 * Handles error checking for building, binding and opening a socket.
 *
 * Returns: file descriptor.
 */
int setup_socket(int port) {

  /* Build */
  int file_descriptor = build_socket();
  if(file_descriptor == -1) {
	  perror_line("Error creating socket");
  }

  /* Bind */
  if(bind_socket(file_descriptor, port) == -1) {
	  perror_line("Error binding socket");
  }
  //DEBUG_PRINT(("Successfully binded socket, fd:%d.\n", file_descriptor));

  /* Listen */
  if(listen(file_descriptor, LISTEN_BACKLOG) == -1) {
	  perror_line("Error listening");
  }
  //DEBUG_PRINT(("Listening successfully, backlog:%d.\n", LISTEN_BACKLOG));

  return file_descriptor;
}
