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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //뮤텍스 선언 및 초기화


void broadcast_message(char* message, int exclude_client) {
    //동시접근 차단
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++)
    {
        if (client_sockets[i] != exclude_client) //보내는 대상 제외 하기 위해
        {
            // 메시지 전송 중 오류 발생 시 클라이언트 소켓 제거
            if (send(client_sockets[i], message, strlen(message), 0) <= 0) {
                printf("Error sending to client %d. Closing connection.\n", client_sockets[i]);
                close(client_sockets[i]);
                for (int j = i; j < client_count - 1; j++) //뒤로 한 칸 씩밀리는 방식으로 client 제거
                {
                    client_sockets[j] = client_sockets[j + 1];
                }
                client_count--;
                i--;  // 배열이 줄어들었으므로 인덱스를 조정
            }
        }
    }
    //접근 제한 해제
    pthread_mutex_unlock(&mutex);
}

void* handle_client(void* arg) {

    int client_socket = *((int*)arg);

    //버퍼 선언
    char buffer[BUFFER_SIZE];

    //수신 바이트수 저장할 변수 선언
    int bytes_received;

    while (1) 
    {
        // 버퍼를 매번 초기화하여 이전 메시지가 남지 않도록 함
        memset(buffer, 0, BUFFER_SIZE);

        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        // 수신된 데이터가 없거나 클라이언트가 연결을 종료한 경우 처리
        if (bytes_received <= 0) {
            if (bytes_received == 0) {

                // 연결이 정상 종료된 경우
                printf("Client on socket %d has disconnected.\n", client_socket);
                break;
            }
            else {
                // 연결이 비정상적으로 종료된 경우
                printf("Error receiving from client %d. Closing connection.\n", client_socket); 
                break;
            }

            // 클라이언트 연결 종료 처리(Mutex로 동시접근 차단)
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; i++) // for문으로 연결 종료 client 찾기
            {
                if (client_sockets[i] == client_socket) {
                    // 연결이 종료된 클라이언트 소켓을 배열에서 제거
                    for (int j = i; j < client_count - 1; j++) //한 칸씩 줄이는 방식으로 소켓 제거
                    {
                        client_sockets[j] = client_sockets[j + 1];
                    }
                    client_count--;
                    break;
                }
            }
            //해제
            pthread_mutex_unlock(&mutex);

            close(client_socket);
            break;  // 연결이 끊긴 경우, 스레드를 종료
        }

        // 수신된 메시지를 다른 클라이언트에게 브로드캐스트
        broadcast_message(buffer, client_socket);
    }

    return NULL;
}


int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    //스레드 관리용(스레드 ID 저장용) tid 설정
    pthread_t tid;

    //브로드 캐스트 설정 키기
    int broadcast_permission = 1;

    //사용법 공지
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // 소켓 생성 (TCP를 위한 SOCK_STREAM 사용)
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("socket error");
        exit(1);
    }

    // 브로드캐스트 옵션 설정
    if (setsockopt(serv_sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission)) < 0) {
        perror("setsockopt() failed");
        close(serv_sock);
        exit(1);
    }

    // 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // 바인딩
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    // 리슨
    if (listen(serv_sock, 5) == -1) {
        perror("listen error");
        exit(1);
    }

    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        // 클라이언트 연결 수락
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        printf("client is connected\n");
        if (clnt_sock == -1) {
            perror("accept error");
            continue;
        }

        // 클라이언트 소켓 배열에 추가
        pthread_mutex_lock(&mutex);   //뮤텍스로 동시접근 제한

        //배열에 clnt_sock 배당
        client_sockets[client_count++] = clnt_sock;

        pthread_mutex_unlock(&mutex); //뮤텍스 해제

        // 클라이언트 처리 스레드 생성
        pthread_create(&tid, NULL, handle_client, (void*)&clnt_sock);

        pthread_detach(tid); //시스템 종료 시 메모리 반환을 위해 메인 스레드와 분리
    }

    close(serv_sock);
    return 0;
}
