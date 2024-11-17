#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "socket_module.h"

#define MAXBUF 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    int clnt_sock = create_socket();
    if (clnt_sock == -1) {
        exit(1);
    }

    struct sockaddr_in serv_addr;
    if (connect_to_server(clnt_sock, argv[1], atoi(argv[2]), &serv_addr) == -1) {
        close(clnt_sock);
        exit(1);
    }

    char buf[MAXBUF];
    printf("서버에 연결되었습니다.\n");

    while (1) {
        memset(buf, 0x00, MAXBUF);
        printf("사용할 서비스를 선택하세요 (1. 회원가입, 2. 로그인, 99. 로그아웃): ");
        fgets(buf, MAXBUF, stdin);
        buf[strcspn(buf, "\n")] = 0;

        write(clnt_sock, buf, strlen(buf) + 1);

        memset(buf, 0x00, MAXBUF);
        ssize_t n = read(clnt_sock, buf, MAXBUF - 1);
        if (n <= 0) {
            printf("서버가 연결을 종료했습니다.\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(clnt_sock);
    return 0;
}
