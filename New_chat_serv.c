#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

#define MAXBUF 1024
#define MAX_ID_LEN 13
#define MAX_PW_LEN 21
#define LEAST_PW_LEN 8

// User 구조체 정의
typedef struct {
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
} User;

// log 파일에 ID와 PW를 저장
void log_user(const char* filename, const char* id, const char* pw) {
    FILE* log_file = fopen(filename, "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "ID: %s PW: %s\n", id, pw);
    fclose(log_file);
}

// log에서 사용자 ID와 PW를 확인하는 함수
int find_user_in_log(const char* filename, const char* id, char* pw) {
    // 파일 포인터를 "file"을 r(읽기 전용 모드)로 열어서 저장
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    char buffer[MAXBUF];
    // 정의된 버퍼의 크기만큼 읽어서 저장 (\n이 나오기 전까지)
    // 읽을 것이 없으면 NULL 반환하는 것을 이용
    while (fgets(buffer, MAXBUF, file) != NULL) {
        char file_id[MAX_ID_LEN], file_pw[21];
        // id, pw 배열 정의

        /* sscanf 함수를 이용해서 버퍼에서 "ID:" 부분을 찾아 그 뒤에 최대 12문자를 "file_id"에,
        "PW" 뒷부분을 찾아 "file_pw"에 저장한다. 성공적으로 저장하면 2를 반환 */
        if (sscanf(buffer, "ID: %12s PW: %20s", file_id, file_pw) == 2) {
            // strcmp를 이용해 file_id와 받은 id를 비교
            if (strcmp(file_id, id) == 0) { // 같은 경우 0을 반환
                if (pw != NULL) { // pw가 NULL이 아닌 경우 (로그인 때 이 함수를 이용해서 pw만 비교하려고)
                    strcpy(pw, file_pw); // file_pw에 저장된 문자열을 pw에 저장
                }
                fclose(file);
                return 1; // 사용자를 찾음
            }
        }
    }

    fclose(file);
    return 0; // 사용자를 찾지 못함
}

// ID가 유효한지 검사하는 함수
int is_valid_id(const char* id) {
    int length = strlen(id); // id의 길이를 저장
    if (length >= MAX_ID_LEN)
        return 0; // 최대 12글자 제한

    for (int i = 0; i < length; i++) { // id 길이 전까지 id라는 배열을
        if (!isalnum(id[i]))
            return 0; // 배열의 i번째 요소가 문자가 알파벳 또는 숫자인지 검사 (영어 숫자로 되어 있으면 1 반환)
    }
    return 1; // 모든 문자가 알파벳 또는 숫자일 경우
}

// PW가 유효한지 검사하는 함수
int is_valid_pw(const char* pw) {
    int length = strlen(pw); // pw의 길이 정의
    if (length < LEAST_PW_LEN)
        return 0; // 최소 8글자 제한

    int has_upper = 0, has_digit = 0, has_special = 0; // 대문자, 숫자, 특수문자가 있는지 체크하는 변수 초기화
    for (int i = 0; i < length; i++) {
        if (isupper(pw[i]))
            has_upper = 1; // pw의 i번째 문자가 대문자인지 아닌지 검사, 하나라도 있다면 1 반환
        else if (isdigit(pw[i]))
            has_digit = 1; // pw의 i번째 문자가 숫자인지 아닌지 검사, 하나라도 있으면 1 반환
        else if (pw[i] == '!' || pw[i] == '@' || pw[i] == '#' || pw[i] == '$') // 특수문자가 있는지 검사
            has_special = 1;
    }
    return has_upper && has_digit && has_special; // 모든 조건을 만족하는지 검사
}

// 메시지 정의
const char* service_message = "사용할 서비스를 선택하세요.\n1. 회원가입\n2. 로그인\n99. 종료\n";
const char* signup_ID_message = "회원가입할 아이디를 입력하시오.";
const char* invalid_id_msg = "유효하지 않은 ID입니다. ID를 다시 입력하세요.\n ID는 알파벳, 숫자로만 구성될 수 있으며, 최대 12자리를 넘지 않아야 합니다.\n";
const char* id_in_use_msg = "이미 사용중인 ID입니다. ID를 다시 입력하세요.\n";
const char* valid_id_msg = "사용 가능한 ID입니다. 패스워드를 입력하세요.\n";
const char* invalid_pw_msg = "PW는 알파벳 대문자, 숫자, 특수문자('!', '@', '#', '$')를 반드시 포함해야 하고, 그 길이는 8자리 이상이어야 합니다.\n";
const char* register_success_msg = "회원가입이 완료되었습니다.\n 사용할 서비스를 선택하세요.\n1. 회원가입\n2. 로그인\n99. 종료\n";
const char* login_ID_message = "아이디를 입력하세요";
const char* id_not_found_msg = "등록되지 않은 ID입니다. 다시 입력하세요.\n";
const char* id_valid_msg = "유효한 ID입니다. 패스워드를 입력하세요.\n";
const char* invalid_signup_pw_msg = "잘못된 PW입니다. ID와 PW가 맞는지 확인하세요.\n";
const char* max_attempt_msg = "잘못된 PW가 5회 입력되었습니다. 연결이 종료됩니다.\n";
const char* login_success_msg = "로그인에 성공하였습니다.\n 사용할 서비스를 선택하세요.\n1. 회원가입\n2. 로그인\n99. 종료";
const char* end_message = "연결이 종료되었습니다.\n";

// 서버와 클라이언트 간의 통신을 로그에 기록하는 함수
void log_communication(const char* log_filename, const char* message) {
    FILE* log_file = fopen(log_filename, "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

// 메인 함수 시작
int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    socklen_t clnt_addr_size;
    int n;
    ssize_t bytes_sent;

    struct sockaddr_in serv_addr, clnt_addr;
    char buf[MAXBUF];
    char log_entry[MAXBUF];

    // 파일이 이미 존재하면 파일에 내용을 추가한다는 뜻의 "a" 모드로 열기
    FILE* db_file = fopen("server_DB.txt", "a");
    if (db_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    FILE* log_file = fopen("server_log.txt", "a");
    if (log_file == NULL) {
        perror("fopen");
        fclose(db_file);
        exit(EXIT_FAILURE);
    }

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("socket() error");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind() error");
        close(serv_sock);
        exit(1);
    }

    if (listen(serv_sock, 5) == -1) {
        perror("listen() error");
        close(serv_sock);
        exit(1);
    }

    // 클라이언트가 연결 요청할 때까지 대기
    while (1) {
        // 클라이언트와의 연결을 허용
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            perror("accept() error");
            continue;
        }

        // 서버 log에 어떤 IP에서 접속했는지 확인
        snprintf(log_entry, MAXBUF, "%s가 접속하였습니다.", inet_ntoa(clnt_addr.sin_addr));
        printf("%s\n", log_entry);
        log_communication("server_log.txt", log_entry);

        // 클라이언트가 사용할 서비스에 대한 안내
        bytes_sent = send(clnt_sock, service_message, strlen(service_message), 0);
        if (bytes_sent != strlen(service_message)) {
            perror("send failed");
            close(clnt_sock);
            continue;
        }
        log_communication("server_log.txt", "클라이언트에게 서비스 안내문을 전송했습니다.");

        while (1) {
            // 클라이언트가 보낸 서비스 선택에 대한 정보 수신
            memset(buf, 0x00, MAXBUF);
            n = recv(clnt_sock, buf, MAXBUF, 0);
            if (n <= 0) { // 반환값이 0인 경우 클라이언트가 연결 종료, 다른 경우는 error이므로 perror로 처리
                if (n == 0) {
                    snprintf(log_entry, MAXBUF, "Client disconnected.");
                    printf("%s\n", log_entry);
                    log_communication("server_log.txt", log_entry);
                }
                else {
                    perror("recv() error");
                }
                close(clnt_sock);
                break;
            }
            buf[n] = '\0'; // 문자열의 올바른 종료를 위해 n번째 비트에 종단문자 삽입
            snprintf(log_entry, MAXBUF, "Received from client: %s", buf);
            log_communication("server_log.txt", log_entry);

            // 회원가입의 경우
            if (strncmp(buf, "1", 1) == 0) {
                // 버퍼 초기화
                memset(buf, 0x00, MAXBUF);

                // 서버 로그에 회원가입 요청했다는 것을 알림
                snprintf(log_entry, MAXBUF, "%s가 회원가입을 요청하였습니다.", inet_ntoa(clnt_addr.sin_addr));
                printf("%s\n", log_entry);
                log_communication("server_log.txt", log_entry);

                // 클라이언트에게 회원가입 아이디를 보내라는 것을 전송
                bytes_sent = send(clnt_sock, signup_ID_message, strlen(signup_ID_message), 0);
                if (bytes_sent != strlen(signup_ID_message)) {
                    perror("send failed");
                    close(clnt_sock);
                    break;
                }
                log_communication("server_log.txt", "회원가입을 위해 ID전송을 요청함.");

                // 클라이언트에게서 아이디를 수신
                n = recv(clnt_sock, buf, MAXBUF, 0);
                buf[n] = '\0';
                char user_id[MAX_ID_LEN];
                strncpy(user_id, buf, MAX_ID_LEN - 1); // 문자열 복사
                user_id[MAX_ID_LEN - 1] = '\0';
                snprintf(log_entry, MAXBUF, "클라이언트로부터 회원가입을 위해 받은 ID: %s", user_id);
                log_communication("server_log.txt", log_entry);

                // 유효한 아이디인지 확인
                while (!is_valid_id(user_id)) {
                    // 유효한 아이디가 아님을 log에 화면에 출력
                    snprintf(log_entry, MAXBUF, "%s가 요청한 %s는 유효하지 않은 ID입니다.", inet_ntoa(clnt_addr.sin_addr), buf);
                    printf("%s\n", log_entry);
                    log_communication("server_log.txt", log_entry);

                    send(clnt_sock, invalid_id_msg, strlen(invalid_id_msg), 0);
                    log_communication("server_log.txt", "클라이언트가 보낸 ID가 유효한 형식이 아닙니다.");

                    // 다시 아이디 수신
                    n = recv(clnt_sock, buf, MAXBUF, 0);
                    buf[n] = '\0';
                    strncpy(user_id, buf, MAX_ID_LEN - 1);
                    user_id[MAX_ID_LEN - 1] = '\0';
                    snprintf(log_entry, MAXBUF, "클라이언트로부터 회원가입을 위해 받은 ID: %s", user_id);
                    log_communication("server_log.txt", log_entry);
                }

                // 이미 사용 중인 id인지 확인
                while (find_user_in_log("server_DB.txt", user_id, NULL)) { // pw에는 NULL을 넣어 PW는 비교 대상에서 제외
                    send(clnt_sock, id_in_use_msg, strlen(id_in_use_msg), 0);
                    log_communication("server_log.txt", "클라이언트가 보낸 ID가 이미 있는 ID입니다.");

                    // ID 다시 수신
                    n = recv(clnt_sock, buf, MAXBUF, 0);
                    buf[n] = '\0';
                    strncpy(user_id, buf, MAX_ID_LEN - 1);
                    user_id[MAX_ID_LEN - 1] = '\0';
                    snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 ID: %s", user_id);
                    log_communication("server_log.txt", log_entry);
                }

                // PW를 전송할 것을 요청
                send(clnt_sock, valid_id_msg, strlen(valid_id_msg), 0);
                log_communication("server_log.txt", "유효한 형식의 ID를 받았습니다. PW를 요청했습니다.");

                // PW 수신
                n = recv(clnt_sock, buf, MAXBUF, 0);
                buf[n] = '\0';
                char user_pw[MAX_PW_LEN];
                strncpy(user_pw, buf, MAX_PW_LEN - 1);
                user_pw[MAX_PW_LEN - 1] = '\0';
                snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 PW: %s", user_pw);
                log_communication("server_log.txt", log_entry);

                // 유효한 pw인지 확인
                while (!is_valid_pw(user_pw)) {
                    // pw의 조건을 알림
                    send(clnt_sock, invalid_pw_msg, strlen(invalid_pw_msg), 0);
                    log_communication("server_log.txt", "유효하지 않은 PW 형식을 클라이언트가 보냈습니다.");

                    // pw를 다시 수신
                    n = recv(clnt_sock, buf, MAXBUF, 0);
                    buf[n] = '\0';
                    strncpy(user_pw, buf, MAX_PW_LEN - 1);
                    user_pw[MAX_PW_LEN - 1] = '\0';
                    snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 PW: %s", user_pw);
                    log_communication("server_log.txt", log_entry);
                }

                // server_DB.txt에 ID와 PW 저장
                log_user("server_DB.txt", user_id, user_pw);

                // 서버 화면에 등록 완료 메시지 출력
                snprintf(log_entry, MAXBUF, "Log> ID/PW : %s/%s가 등록 되었습니다.", user_id, user_pw);
                printf("%s\n", log_entry);
                log_communication("server_log.txt", log_entry);

                // 클라이언트에게 회원가입 완료 메시지 출력 및 서비스 선택
                send(clnt_sock, register_success_msg, strlen(register_success_msg), 0);
                log_communication("server_log.txt", "회원가입 성공메시지를 보냈습니다.");
            }

            // 로그인 경우
            else if (strncmp(buf, "2", 1) == 0) {
                // 버퍼 초기화
                memset(buf, 0x00, MAXBUF);

                // 무엇을 요청했는지 출력
                snprintf(log_entry, MAXBUF, "%s가 로그인을 요청하였습니다.", inet_ntoa(clnt_addr.sin_addr));
                printf("%s\n", log_entry);
                log_communication("server_log.txt", log_entry);

                // 아이디 입력을 요청
                bytes_sent = send(clnt_sock, login_ID_message, strlen(login_ID_message), 0);
                if (bytes_sent != strlen(login_ID_message)) {
                    perror("send failed");
                    close(clnt_sock);
                    break;
                }
                log_communication("server_log.txt", "ID요청을 클라이언트에게 보냈습니다.");

                // 아이디 수신
                n = recv(clnt_sock, buf, MAXBUF, 0);
                buf[n] = '\0';
                char user_id[MAX_ID_LEN];
                strncpy(user_id, buf, MAX_ID_LEN - 1);
                user_id[MAX_ID_LEN - 1] = '\0';
                snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 ID: %s", user_id);
                log_communication("server_log.txt", log_entry);

                // pw 배열 정의
                char stored_pw[21];
                while (!find_user_in_log("server_DB.txt", user_id, stored_pw)) { // find_user_in_log를 통해 stored_pw에 저장된 pw 저장
                    // 등록되지 않은 ID임을 알림
                    send(clnt_sock, id_not_found_msg, strlen(id_not_found_msg), 0);
                    log_communication("server_log.txt", "등록되지 않은 ID입니다.");

                    // 유효하지 않은 아이디임을 출력
                    snprintf(log_entry, MAXBUF, "%s가 요청한 ID(\"%s\")는 등록되지 않은 ID입니다.", inet_ntoa(clnt_addr.sin_addr), user_id);
                    printf("%s\n", log_entry);
                    log_communication("server_log.txt", log_entry);

                    // 다시 아이디 수신
                    n = recv(clnt_sock, buf, MAXBUF, 0);
                    buf[n] = '\0';
                    strncpy(user_id, buf, MAX_ID_LEN - 1);
                    user_id[MAX_ID_LEN - 1] = '\0';
                    snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 아이디: %s", user_id);
                    log_communication("server_log.txt", log_entry);
                }

                // Pw 입력을 요청
                send(clnt_sock, id_valid_msg, strlen(id_valid_msg), 0);
                log_communication("server_log.txt", "유효한 ID를 보냈습니다. PW를 요청합니다.");

                // PW 수신
                n = recv(clnt_sock, buf, MAXBUF, 0);
                buf[n] = '\0';
                char user_pw[MAX_PW_LEN];
                strncpy(user_pw, buf, MAX_PW_LEN - 1);
                user_pw[MAX_PW_LEN - 1] = '\0';
                snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 비밀번호: %s", user_pw);
                log_communication("server_log.txt", log_entry);

                // 시도가 몇 번인지 알기 위한 변수 저장
                int attempt = 0;
                // 위의 함수에서 저장된 pw와 받은 pw 비교
                while (strcmp(stored_pw, user_pw) != 0) {
                    if (attempt < 5) { // 시도가 5번 이하면 PW를 받아줌
                        // 잘못된 PW임을 송신
                        send(clnt_sock, invalid_signup_pw_msg, strlen(invalid_signup_pw_msg), 0);
                        log_communication("server_log.txt", "클라이언트가 유효하지 않은 pw를 보냈습니다.");

                        // 시도 횟수 카운팅
                        attempt++;
                    }
                    // 시도가 5번 초과인 경우 연결 종료
                    else {
                        // 연결 종료 메시지 클라이언트에게 송신
                        send(clnt_sock, max_attempt_msg, strlen(max_attempt_msg), 0);
                        log_communication("server_log.txt", "시도 횟수가 5회를 초과하였습니다. <종료>");

                        // 서버 종료 및 루프 탈출
                        close(clnt_sock);
                        break;
                    }

                    // PW 수신
                    n = recv(clnt_sock, buf, MAXBUF, 0);
                    buf[n] = '\0';
                    strncpy(user_pw, buf, MAX_PW_LEN - 1);
                    user_pw[MAX_PW_LEN - 1] = '\0';
                    snprintf(log_entry, MAXBUF, "클라이언트로부터 받은 비밀번호: %s", user_pw);
                    log_communication("server_log.txt", log_entry);
                }

                // 시도가 5번 아래인 경우
                if (attempt < 5) {
                    // log 성공 메시지 전송
                    send(clnt_sock, login_success_msg, strlen(login_success_msg), 0);
                    log_communication("server_log.txt", "로그인 성공메세지를 보냈습니다.");
                }
            }

            // 종료의 경우
            else if (strncmp(buf, "99", 2) == 0) {
                // 연결 종료를 요청했음을 알림
                snprintf(log_entry, MAXBUF, "클라이언트가 연결 종료를 요청했습니다.");
                printf("%s\n", log_entry);
                log_communication("server_log.txt", log_entry);

                // 연결 종료문 안내
                send(clnt_sock, end_message, strlen(end_message), 0);
                log_communication("server_log.txt", "연결 종료문을 보냈습니다. <종료>");

                // 연결 종료
                close(clnt_sock);
                break;
            }
        }
    }

    close(serv_sock);
    fclose(db_file);
    fclose(log_file);
    return 0;
}
