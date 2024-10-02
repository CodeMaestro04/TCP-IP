#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void* receive_messages(void* arg);

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t tid;
    char message[BUFFER_SIZE];

    // ���� ����
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket error");
        exit(1);
    }

    // ���� �ּ� ����
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // ���� ����
    server_addr.sin_port = htons(9000);  // ������ ��Ʈ ��ȣ 9000

    // ������ ����
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect error");
        exit(1);
    }

    printf("Entered Chatroom.\n");

    // �޽��� ���� ������ ����
    pthread_create(&tid, NULL, receive_messages, (void*)&client_socket);

    // �޽��� �Է� �� ����
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);

        // "/quit" �Է� �� ���� ����
        if (strncmp(message, "/quit", 5) == 0) {
            printf("Disconnecting from the server...\n");
            break;
        }

        send(client_socket, message, strlen(message), 0);
    }

    // ���� ����
    close(client_socket);
    printf("Disconnected.\n");

    return 0;
}

void* receive_messages(void* arg) {
    //���� �޾ƿ���
    int client_socket = *((int*)arg);

    //���� ����
    char buffer[BUFFER_SIZE];

    //���� ����Ʈ �� ���� ����
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0'; //���ڿ� ���� ���� ����
        printf("%s", buffer);
    }

    return NULL;
}