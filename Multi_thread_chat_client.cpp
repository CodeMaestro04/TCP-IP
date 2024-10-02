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

    // 소켓 생성
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket error");
        exit(1);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 로컬 서버
    server_addr.sin_port = htons(9000);  // 서버의 포트 번호 9000

    // 서버에 연결
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect error");
        exit(1);
    }

    printf("Entered Chatroom.\n");

    // 메시지 수신 스레드 생성
    pthread_create(&tid, NULL, receive_messages, (void*)&client_socket);

    // 메시지 입력 및 전송
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);

        // "/quit" 입력 시 연결 종료
        if (strncmp(message, "/quit", 5) == 0) {
            printf("Disconnecting from the server...\n");
            break;
        }

        send(client_socket, message, strlen(message), 0);
    }

    // 연결 종료
    close(client_socket);
    printf("Disconnected.\n");

    return 0;
}

void* receive_messages(void* arg) {
    //소켓 받아오기
    int client_socket = *((int*)arg);

    //버퍼 선언
    char buffer[BUFFER_SIZE];

    //받을 바이트 수 저장 변수
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0'; //문자열 정상 종료 위해
        printf("%s", buffer);
    }

    return NULL;
}