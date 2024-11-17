// socket_module.c
#include "socket_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int create_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket() error");
    }
    return sock;
}

int connect_to_server(int sock, const char* ip, int port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)addr, sizeof(*addr)) == -1) {
        perror("connect() error");
        return -1;
    }
    return 0;
}

int bind_and_listen(int sock, int port) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind() error");
        return -1;
    }
    if (listen(sock, 5) == -1) {
        perror("listen() error");
        return -1;
    }
    return 0;
}