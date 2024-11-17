#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "log_module.h"
#include "user_module.h"  // ����: user_module.h�� �����Ͽ� �Լ� ������ �ν��ϰ� ��
#include "socket_module.h"

#define MAXBUF 1024

void handle_client(int clnt_sock) {
    char buf[MAXBUF];
    char user_id[MAX_ID_LEN];
    char user_pw[MAX_PW_LEN];
    int is_logged_in = 0;

    const char* service_message = "����� ���񽺸� �����ϼ���.\n1. ȸ������\n2. �α���\n99. ����\n";
    const char* signup_ID_message = "ȸ�������� ���̵� �Է��Ͻÿ�: ";
    const char* invalid_id_msg = "��ȿ���� ���� ID�Դϴ�. �ٽ� �Է��ϼ���.\n";
    const char* id_in_use_msg = "�̹� ������� ID�Դϴ�. �ٽ� �Է��ϼ���.\n";
    const char* valid_id_msg = "��� ������ ID�Դϴ�. �н����带 �Է��ϼ���: ";
    const char* invalid_pw_msg = "PW�� �빮��, ����, Ư�����ڸ� �����ؾ� �ϸ� 8�ڸ� �̻��̾�� �մϴ�.\n";
    const char* register_success_msg = "ȸ�������� �Ϸ�Ǿ����ϴ�.\n";
    const char* login_ID_message = "���̵� �Է��ϼ���: ";
    const char* id_not_found_msg = "��ϵ��� ���� ID�Դϴ�. �ٽ� �Է��ϼ���.\n";
    const char* id_valid_msg = "ID�� Ȯ�εǾ����ϴ�. �н����带 �Է��ϼ���: ";
    const char* login_fail_msg = "�߸��� PW�Դϴ�. �ٽ� �õ��ϼ���.\n";
    const char* max_attempt_msg = "5ȸ �����Ͽ����ϴ�. ������ ����˴ϴ�.\n";
    const char* login_success_msg = "�α��ο� �����Ͽ����ϴ�.\n";
    const char* logout_message = "�α׾ƿ� �Ǿ����ϴ�.\n";
    const char* end_message = "������ ����Ǿ����ϴ�.\n";

    int attempt = 0;

    while (1) {
        write(clnt_sock, service_message, strlen(service_message));

        memset(buf, 0x00, MAXBUF);
        ssize_t n = read(clnt_sock, buf, MAXBUF - 1);
        if (n <= 0) {
            printf("Ŭ���̾�Ʈ�� ������ �����߽��ϴ�.\n");
            break;
        }
        buf[n] = '\0';

        if (strncmp(buf, "1", 1) == 0) { // ȸ������
            write(clnt_sock, signup_ID_message, strlen(signup_ID_message));
            read(clnt_sock, user_id, MAX_ID_LEN);
            if (!is_valid_id(user_id)) {
                write(clnt_sock, invalid_id_msg, strlen(invalid_id_msg));
                continue;
            }
            if (find_user_in_log("server_DB.txt", user_id, NULL)) {
                write(clnt_sock, id_in_use_msg, strlen(id_in_use_msg));
                continue;
            }

            write(clnt_sock, valid_id_msg, strlen(valid_id_msg));
            read(clnt_sock, user_pw, MAX_PW_LEN);
            if (!is_valid_pw(user_pw)) {
                write(clnt_sock, invalid_pw_msg, strlen(invalid_pw_msg));
                continue;
            }

            log_user("server_DB.txt", user_id, user_pw);
            write(clnt_sock, register_success_msg, strlen(register_success_msg));
        }
        else if (strncmp(buf, "2", 1) == 0) { // �α���
            if (is_logged_in) {
                write(clnt_sock, "�̹� �α��εǾ� �ֽ��ϴ�.\n", strlen("�̹� �α��εǾ� �ֽ��ϴ�.\n"));
                continue;
            }

            write(clnt_sock, login_ID_message, strlen(login_ID_message));
            read(clnt_sock, user_id, MAX_ID_LEN);

            char stored_pw[MAX_PW_LEN];
            if (!find_user_in_log("server_DB.txt", user_id, stored_pw)) {
                write(clnt_sock, id_not_found_msg, strlen(id_not_found_msg));
                continue;
            }

            write(clnt_sock, id_valid_msg, strlen(id_valid_msg));
            read(clnt_sock, user_pw, MAX_PW_LEN);

            attempt = 0;
            while (strcmp(stored_pw, user_pw) != 0) {
                attempt++;
                if (attempt >= 5) {
                    write(clnt_sock, max_attempt_msg, strlen(max_attempt_msg));
                    close(clnt_sock);
                    return;
                }
                write(clnt_sock, login_fail_msg, strlen(login_fail_msg));
                read(clnt_sock, user_pw, MAX_PW_LEN);
            }

            is_logged_in = 1;
            write(clnt_sock, login_success_msg, strlen(login_success_msg));
        }
        else if (strncmp(buf, "99", 2) == 0) { // �α׾ƿ�
            if (is_logged_in) {
                is_logged_in = 0;
                write(clnt_sock, logout_message, strlen(logout_message));
            }
            else {
                write(clnt_sock, "�α��ε� ���°� �ƴմϴ�.\n", strlen("�α��ε� ���°� �ƴմϴ�.\n"));
            }
            write(clnt_sock, end_message, strlen(end_message));
            close(clnt_sock);
            break;
        }
        else {
            write(clnt_sock, "��ȿ���� ���� �ɼ��Դϴ�.\n", strlen("��ȿ���� ���� �ɼ��Դϴ�.\n"));
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int serv_sock = create_socket();
    if (serv_sock == -1) {
        exit(1);
    }

    int port = atoi(argv[1]);
    if (bind_and_listen(serv_sock, port) == -1) {
        close(serv_sock);
        exit(1);
    }

    printf("Server is listening on port %d...\n", port);

    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    while (1) {
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            perror("accept() error");
            continue;
        }
        printf("Client connected: %s\n", inet_ntoa(clnt_addr.sin_addr));
        handle_client(clnt_sock);
    }

    close(serv_sock);
    return 0;
}
