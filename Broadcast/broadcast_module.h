// broadcast_module.h
#ifndef BROADCAST_MODULE_H
#define BROADCAST_MODULE_H

#include <pthread.h>

// 브로드캐스트 메시지 함수 선언
void broadcast_message(char* message, int exclude_client, int* client_sockets, int* client_count, pthread_mutex_t* mutex);

#endif
