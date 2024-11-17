// client_module.c
#include "client_module.h"
#include "broadcast_module.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

extern int client_sockets[];
extern int client_count;
extern pthread_mutex_t mutex;

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client on socket %d has disconnected.\n", client_socket);
            } else {
                printf("Error receiving from client %d. Closing connection.\n", client_socket);
            }

            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; i++) {
                if (client_sockets[i] == client_socket) {
                    for (int j = i; j < client_count - 1; j++) {
                        client_sockets[j] = client_sockets[j + 1];
                    }
                    client_count--;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);

            close(client_socket);
            break;
        }

        broadcast_message(buffer, client_socket, client_sockets, &client_count, &mutex);
    }

    return NULL;
}
