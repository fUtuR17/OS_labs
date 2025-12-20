#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "common.h"

SharedData *data;
char my_login[LOGIN_LEN];
int my_index;

void clear_screen() {
    printf("\033[2J\033[H");
}

void header() {
    printf("=====================================\n");
    printf(" IPC CHAT | user: %s\n", my_login);
    printf("=====================================\n");
}

/* ===== Receiver thread ===== */

void *receiver(void *arg) {
    while (1) {
        sem_wait(&data->mutex);

        while (data->clients[my_index].last_read < data->message_count) {
            int i = data->clients[my_index].last_read;
            Message *msg = &data->messages[i];

            if (strcmp(msg->to, my_login) == 0) {
                printf("\n[%s]: %s\n", msg->from, msg->text);
            }

            data->clients[my_index].last_read++;
        }

        sem_post(&data->mutex);
        usleep(200000);
    }
    return NULL;
}


void show_all_incoming() {
    clear_screen();
    header();
    printf("All incoming messages:\n\n");

    sem_wait(&data->mutex);

    int found = 0;
    for (int i = 0; i < data->message_count; i++) {
        Message *msg = &data->messages[i];
        if (strcmp(msg->to, my_login) == 0) {
            printf("[%s]: %s\n", msg->from, msg->text);
            found = 1;
        }
    }

    sem_post(&data->mutex);

    if (!found) {
        printf("No messages.\n");
    }

    printf("\nPress Enter...");
    getchar();
}


void search_messages() {
    char key[TEXT_LEN];

    printf("Keyword: ");
    fgets(key, TEXT_LEN, stdin);
    key[strcspn(key, "\n")] = 0;

    clear_screen();
    header();
    printf("Search results:\n\n");

    sem_wait(&data->mutex);

    int found = 0;
    for (int i = 0; i < data->message_count; i++) {
        Message *msg = &data->messages[i];
        if ((strcmp(msg->to, my_login) == 0 ||
             strcmp(msg->from, my_login) == 0) &&
            strstr(msg->text, key)) {

            printf("[%s -> %s]: %s\n",
                   msg->from, msg->to, msg->text);
            found = 1;
        }
    }

    sem_post(&data->mutex);

    if (!found) printf("Nothing found.\n");

    printf("\nPress Enter...");
    getchar();
}

int main() {
    clear_screen();
    printf("Login: ");
    scanf("%31s", my_login);
    getchar();

    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open");
        return 1;
    }

    data = mmap(NULL, sizeof(SharedData),
                PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, 0);

    sem_wait(&data->mutex);

    my_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!data->clients[i].active) {
            strcpy(data->clients[i].login, my_login);
            data->clients[i].active = 1;
            data->clients[i].last_read = data->message_count;
            my_index = i;
            break;
        }
    }

    sem_post(&data->mutex);

    if (my_index == -1) {
        printf("Server full\n");
        return 1;
    }

    pthread_t t;
    pthread_create(&t, NULL, receiver, NULL);

    while (1) {
        clear_screen();
        header();

        printf("1) Send message\n");
        printf("2) Search messages\n");
        printf("3) Show all incoming\n");
        printf("4) Exit\n");
        printf("Choose: ");

        int c;
        scanf("%d", &c);
        getchar();

        if (c == 1) {
            char to[LOGIN_LEN], text[TEXT_LEN];

            printf("To: ");
            scanf("%31s", to);
            getchar();

            printf("Message: ");
            fgets(text, TEXT_LEN, stdin);
            text[strcspn(text, "\n")] = 0;

            sem_wait(&data->mutex);

            if (data->message_count < MAX_MESSAGES) {
                Message *m = &data->messages[data->message_count++];
                strcpy(m->from, my_login);
                strcpy(m->to, to);
                strcpy(m->text, text);
            }

            sem_post(&data->mutex);

        } else if (c == 2) {
            search_messages();

        } else if (c == 3) {
            show_all_incoming();

        } else if (c == 4) {
            sem_wait(&data->mutex);
            data->clients[my_index].active = 0;
            sem_post(&data->mutex);
            exit(0);
        }
    }
}
