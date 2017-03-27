#include "test_client.h"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../source/socket_helper.h"
#include "../source/parser.h"
#include "../source/message_manager.h"

pthread_t control_thread;
pthread_t data_thread;

/**
 * A simple assert function, test to see if 2 strings are equal.
 */
void my_assert_equals(char* a, char* b, char* test_name) {
    int test_success = strcmp(a, b);
    if(test_success) {
        char o[512];
        sprintf(o, "ASSERTION FAILED: %s: ('%s' != '%s')", test_name, a, b);
        perror_exit(o);
    } else {
        DEBUG_PRINT(("> Passed: '%s'\n", test_name));
    }
    fflush(stdout);
}

/**
 * Send a command (input) to the connection.
 */
void send_cmd(char* input, int connection) {
    if(write(connection, input, strlen(input) + 1) < 0) perror_exit("Write error.");
}

/**
 * Test a command with a result from a connection.
 */
void test_cmd(char* cmd, char* res, char* test_name, int connection) {
    char output[LINE] = {0};
    send_cmd(cmd, connection);
    read_message(connection, output);
    my_assert_equals(output, res, test_name);
}

/**
 * Close the connection to the server.
 */
void leave_server(int connection) {
    close(connection);
}

/**
 * Connect to a server on a port.
 */
int connect_to_server(int port) {
    int my_test_sock = socket(AF_INET, SOCK_STREAM, 0); if(my_test_sock == -1) perror_exit("Error opening socket");

    struct in_addr server_addr;
    if(!inet_pton(AF_INET, "127.0.0.1", &server_addr)) perror_exit("inet_aton");

    struct sockaddr_in connection;
    memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
    connection.sin_family = AF_INET;
    connection.sin_port = htons(port);

    if (connect(my_test_sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perror_exit("Have you started the server?");

    char output[LINE] = {0};
    read_message(my_test_sock, output); // Receive Motd
    return my_test_sock;
    // int sock = build_socket();
    // if(sock == -1) perror_exit("Cant build socket!");
    //
    // int sockfd = 0, n = 0;
    // char recvBuff[1024];
    // struct sockaddr_in serv_addr;
    //
    // memset(recvBuff, '0',sizeof(recvBuff));
    // if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    // {
    //     printf("\n Error : Could not create socket \n");
    //     return 1;
    // }
    //
    // memset(&serv_addr, '0', sizeof(serv_addr));
    //
    // serv_addr.sin_family = AF_INET;
    // serv_addr.sin_port = htons(port);
    //
    // if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    // {
    //     printf("\n inet_pton error occured\n");
    //     return 1;
    // }
    //
    // if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    // {
    //    printf("\n Error : Connect Failed \n");
    //    return 1;
    // }


    //
    // struct sockaddr_in sa;
    // socklen_t len = sizeof(sa);
    // memset(&sa, 0, len);
    // sa.sin_family = AF_INET;
    // sa.sin_port = htons(port);
    //
    // char* addr = "127.0.0.1";
    // if(1 != inet_pton(AF_INET, addr, &(sa.sin_addr))) perror_exit("inet error");
    //
    // int result = connect(sock, &sa, len);
    // if(result == -1) perror_exit("Tests cant connect to server...");

    // char client_message[LINE];
    // memset(client_message, 0, LINE);
    // int read_size = read_message(sockfd, &client_message);
    // client_message[read_size] = '\0';
    // return sockfd;
}
