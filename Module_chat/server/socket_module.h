#ifndef SOCKET_MODULE_H
#define SOCKET_MODULE_H

#include <netinet/in.h>

// Function to create a socket
int create_socket();

// Function to connect to a server
int connect_to_server(int sock, const char* ip, int port, struct sockaddr_in* addr);

// Function to bind and listen on a socket
int bind_and_listen(int sock, int port);

#endif // SOCKET_MODULE_H
