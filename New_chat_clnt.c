#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXBUF 1024

int main(int argc, char* argv[]) {
    struct sockaddr_in clnt_addr;
    int clnt_sock;
    socklen_t clnt_len;
    char buf[MAXBUF];
    ssize_t bytes_received;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    if ((clnt_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error: ");
        return 1;
    }

    clnt_addr.sin_family = AF_INET;
    clnt_addr.sin_addr.s_addr = inet_addr(argv[1]);
    clnt_addr.sin_port = htons(atoi(argv[2]));

    clnt_len = sizeof(clnt_addr);

    if (connect(clnt_sock, (struct sockaddr*)&clnt_addr, clnt_len) == -1) {
        perror("connect() error: ");
        return 1;
    }

    // 수신 및 응답 무한 루프
    while (1) {
        // 서버로부터의 응답 읽기
        memset(buf, 0x00, MAXBUF);
        bytes_received = read(clnt_sock, buf, MAXBUF - 1);
        if (bytes_received <= 0) {
            printf("서버와의 연결이 종료되었습니다.\n");
            break;
        }
        buf[bytes_received] = '\0';
        printf("서버> %s\n", buf);

        // 메시지 전송
        memset(buf, 0x00, MAXBUF);
        printf("Server 에게 보내는 메시지: ");
        fgets(buf, MAXBUF, stdin);
        buf[strcspn(buf, "\n")] = 0; // 개행 문자 제거

        if (strncmp(buf, "q", 1) == 0) {
            break;
        }

        write(clnt_sock, buf, strlen(buf) + 1);

        if (strncmp(buf, "99", 2) == 0) {
            printf("연결이 종료됩니다.\n");
            close(clnt_sock);
            break;
        }
    }

    close(clnt_sock);
    return 0;
}
