// broadcast_module.c
#include "broadcast_module.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void broadcast_message(char* message, int exclude_client, int* client_sockets, int* client_count, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex); // 동시 접근 차단
    for (int i = 0; i < *client_count; i++) {
        if (client_sockets[i] != exclude_client) { // 보내는 대상 제외
            // 메시지 전송 중 오류 발생 시 클라이언트 소켓 제거
            if (send(client_sockets[i], message, strlen(message), 0) <= 0) {
                printf("Error sending to client %d. Closing connection.\n", client_sockets[i]);
                close(client_sockets[i]);
                for (int j = i; j < *client_count - 1; j++) {
                    client_sockets[j] = client_sockets[j + 1];
                }
                (*client_count)--;
                i--; // 배열이 줄어들었으므로 인덱스를 조정
            }
        }
    }
    pthread_mutex_unlock(mutex); // 접근 제한 해제
}
