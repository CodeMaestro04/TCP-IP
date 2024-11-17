#include "user_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_valid_id(const char* id) {
    int length = strlen(id);
    if (length >= MAX_ID_LEN)
        return 0;

    for (int i = 0; i < length; i++) {
        if (!isalnum(id[i]))
            return 0;
    }
    return 1;
}

int is_valid_pw(const char* pw) {
    int length = strlen(pw);
    if (length < LEAST_PW_LEN)
        return 0;

    int has_upper = 0, has_digit = 0, has_special = 0;
    for (int i = 0; i < length; i++) {
        if (isupper(pw[i]))
            has_upper = 1;
        else if (isdigit(pw[i]))
            has_digit = 1;
        else if (pw[i] == '!' || pw[i] == '@' || pw[i] == '#' || pw[i] == '$')
            has_special = 1;
    }
    return has_upper && has_digit && has_special;
}

int find_user_in_log(const char* filename, const char* id, char* pw) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    char buffer[1024];
    while (fgets(buffer, 1024, file) != NULL) {
        char file_id[MAX_ID_LEN], file_pw[MAX_PW_LEN];
        if (sscanf(buffer, "ID: %12s PW: %20s", file_id, file_pw) == 2) {
            if (strcmp(file_id, id) == 0) {
                if (pw != NULL) {
                    strcpy(pw, file_pw);
                }
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 0;
}

void log_user(const char* filename, const char* id, const char* pw) {
    FILE* log_file = fopen(filename, "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "ID: %s PW: %s\n", id, pw);
    fclose(log_file);
}

