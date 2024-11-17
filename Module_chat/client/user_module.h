#ifndef USER_MODULE_H
#define USER_MODULE_H

#define MAX_ID_LEN 13
#define MAX_PW_LEN 21
#define LEAST_PW_LEN 8

// User 구조체 정의
typedef struct {
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
} User;

// 함수 선언
int is_valid_id(const char* id);
int is_valid_pw(const char* pw);
int find_user_in_log(const char* filename, const char* id, char* pw);
void log_user(const char* filename, const char* id, const char* pw);

#endif // USER_MODULE_H
