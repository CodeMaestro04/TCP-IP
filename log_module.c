// log_module.c
#include "log_module.h"
#include <stdio.h>
#include <stdlib.h>

void log_message(const char* filename, const char* message) {
    FILE* log_file = fopen(filename, "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

