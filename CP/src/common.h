#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>

#define SHM_NAME "/chat_shm"

#define MAX_CLIENTS 10
#define MAX_MESSAGES 500
#define LOGIN_LEN 32
#define TEXT_LEN 256

typedef struct {
    char from[LOGIN_LEN];
    char to[LOGIN_LEN];
    char text[TEXT_LEN];
} Message;

typedef struct {
    char login[LOGIN_LEN];
    int active;
    int last_read;
} Client;

typedef struct {
    Client clients[MAX_CLIENTS];
    Message messages[MAX_MESSAGES];
    int message_count;
    sem_t mutex;
} SharedData;

#endif
