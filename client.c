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
    printf("������ ����Ǿ����ϴ�.\n");

    while (1) {
        memset(buf, 0x00, MAXBUF);
        printf("����� ���񽺸� �����ϼ��� (1. ȸ������, 2. �α���, 99. �α׾ƿ�): ");
        fgets(buf, MAXBUF, stdin);
        buf[strcspn(buf, "\n")] = 0;

        write(clnt_sock, buf, strlen(buf) + 1);

        memset(buf, 0x00, MAXBUF);
        ssize_t n = read(clnt_sock, buf, MAXBUF - 1);
        if (n <= 0) {
            printf("������ ������ �����߽��ϴ�.\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(clnt_sock);
    return 0;
}
