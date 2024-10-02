#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>       

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //���ؽ� ���� �� �ʱ�ȭ


void broadcast_message(char* message, int exclude_client) {
    //�������� ����
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++)
    {
        if (client_sockets[i] != exclude_client) //������ ��� ���� �ϱ� ����
        {
            // �޽��� ���� �� ���� �߻� �� Ŭ���̾�Ʈ ���� ����
            if (send(client_sockets[i], message, strlen(message), 0) <= 0) {
                printf("Error sending to client %d. Closing connection.\n", client_sockets[i]);
                close(client_sockets[i]);
                for (int j = i; j < client_count - 1; j++) //�ڷ� �� ĭ ���и��� ������� client ����
                {
                    client_sockets[j] = client_sockets[j + 1];
                }
                client_count--;
                i--;  // �迭�� �پ������Ƿ� �ε����� ����
            }
        }
    }
    //���� ���� ����
    pthread_mutex_unlock(&mutex);
}

void* handle_client(void* arg) {

    int client_socket = *((int*)arg);

    //���� ����
    char buffer[BUFFER_SIZE];

    //���� ����Ʈ�� ������ ���� ����
    int bytes_received;

    while (1) 
    {
        // ���۸� �Ź� �ʱ�ȭ�Ͽ� ���� �޽����� ���� �ʵ��� ��
        memset(buffer, 0, BUFFER_SIZE);

        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        // ���ŵ� �����Ͱ� ���ų� Ŭ���̾�Ʈ�� ������ ������ ��� ó��
        if (bytes_received <= 0) {
            if (bytes_received == 0) {

                // ������ ���� ����� ���
                printf("Client on socket %d has disconnected.\n", client_socket);
                break;
            }
            else {
                // ������ ������������ ����� ���
                printf("Error receiving from client %d. Closing connection.\n", client_socket); 
                break;
            }

            // Ŭ���̾�Ʈ ���� ���� ó��(Mutex�� �������� ����)
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; i++) // for������ ���� ���� client ã��
            {
                if (client_sockets[i] == client_socket) {
                    // ������ ����� Ŭ���̾�Ʈ ������ �迭���� ����
                    for (int j = i; j < client_count - 1; j++) //�� ĭ�� ���̴� ������� ���� ����
                    {
                        client_sockets[j] = client_sockets[j + 1];
                    }
                    client_count--;
                    break;
                }
            }
            //����
            pthread_mutex_unlock(&mutex);

            close(client_socket);
            break;  // ������ ���� ���, �����带 ����
        }

        // ���ŵ� �޽����� �ٸ� Ŭ���̾�Ʈ���� ��ε�ĳ��Ʈ
        broadcast_message(buffer, client_socket);
    }

    return NULL;
}


int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    //������ ������(������ ID �����) tid ����
    pthread_t tid;

    //��ε� ĳ��Ʈ ���� Ű��
    int broadcast_permission = 1;

    //���� ����
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // ���� ���� (TCP�� ���� SOCK_STREAM ���)
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("socket error");
        exit(1);
    }

    // ��ε�ĳ��Ʈ �ɼ� ����
    if (setsockopt(serv_sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission)) < 0) {
        perror("setsockopt() failed");
        close(serv_sock);
        exit(1);
    }

    // ���� �ּ� ����
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // ���ε�
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    // ����
    if (listen(serv_sock, 5) == -1) {
        perror("listen error");
        exit(1);
    }

    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        // Ŭ���̾�Ʈ ���� ����
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        printf("client is connected\n");
        if (clnt_sock == -1) {
            perror("accept error");
            continue;
        }

        // Ŭ���̾�Ʈ ���� �迭�� �߰�
        pthread_mutex_lock(&mutex);   //���ؽ��� �������� ����

        //�迭�� clnt_sock ���
        client_sockets[client_count++] = clnt_sock;

        pthread_mutex_unlock(&mutex); //���ؽ� ����

        // Ŭ���̾�Ʈ ó�� ������ ����
        pthread_create(&tid, NULL, handle_client, (void*)&clnt_sock);

        pthread_detach(tid); //�ý��� ���� �� �޸� ��ȯ�� ���� ���� ������� �и�
    }

    close(serv_sock);
    return 0;
}