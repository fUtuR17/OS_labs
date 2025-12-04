#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_CHAR 256
#define SHARED_MEM_SIZE 4096
#define SHM_NAME "/string_processor"
#define MAX_STRINGS 100

typedef struct {
    char data[MAX_STRINGS][SHARED_MEM_SIZE];
    size_t lengths[MAX_STRINGS];
    int processed_by_child1[MAX_STRINGS];
    int processed_by_child2[MAX_STRINGS];
    int count;
    int finished;
} shared_data_t;

int main(){
    char filename1[MAX_CHAR];
    char filename2[MAX_CHAR];

    printf("Введите имя 1-го файла\n");
    if (fgets(filename1, sizeof(filename1), stdin) == NULL) {
        perror("filename1 error");
        exit(1);
    }
    printf("Введите имя 2-го файла\n");
    if (fgets(filename2, sizeof(filename2), stdin) == NULL) {
        perror("filename2 error");
        exit(1);
    }
    
    filename1[strcspn(filename1, "\n")] = '\0';
    filename2[strcspn(filename2, "\n")] = '\0';

    // Создаем shared memory объект
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open error");
        exit(1);
    }

    // Устанавливаем размер shared memory
    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
        perror("ftruncate error");
        exit(1);
    }

    // Отображаем shared memory в адресное пространство
    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), 
                                     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap error");
        exit(1);
    }

    // Инициализируем shared data
    shared_data->count = 0;
    shared_data->finished = 0;
    for (int i = 0; i < MAX_STRINGS; i++) {
        shared_data->processed_by_child1[i] = 0;
        shared_data->processed_by_child2[i] = 0;
    }

    pid_t pid1, pid2;

    // Первый дочерний процесс
    switch(pid1 = fork()) {
        case -1:
            perror("fork 1 error");
            exit(1);
        case 0:
            execl("./child1", "./child1", filename1, SHM_NAME, NULL);
            perror("execl child 1 error");
            exit(1);
    }

    // Второй дочерний процесс
    switch(pid2 = fork()) {
        case -1:
            perror("fork 2 error");
            exit(1);
        case 0:
            execl("./child2", "./child2", filename2, SHM_NAME, NULL);
            perror("execl child 2 error");
            exit(1);
    }

    // Родительский процесс - ввод данных
    char *buffer = NULL;
    size_t size = 0;
    
    while (1) {
        printf("Введите строку или 'exit' для выхода:\n");
        fflush(stdout);
        ssize_t len = getline(&buffer, &size, stdin);
        if (len == -1) break;

        if (strcmp(buffer, "exit\n") == 0) {
            break;
        }

        // Добавляем строку в shared memory
        if (shared_data->count < MAX_STRINGS) {
            int idx = shared_data->count;
            strncpy(shared_data->data[idx], buffer, SHARED_MEM_SIZE - 1);
            shared_data->data[idx][SHARED_MEM_SIZE - 1] = '\0';
            shared_data->lengths[idx] = len;
            shared_data->count++;
        } else {
            printf("Превышен лимит строк!\n");
        }
    }

    // Сигнализируем о завершении
    shared_data->finished = 1;

    free(buffer);

    // Ожидаем завершения дочерних процессов
    wait(NULL);
    wait(NULL);

    // Освобождаем ресурсы
    munmap(shared_data, sizeof(shared_data_t));
    shm_unlink(SHM_NAME);
    
    return 0;
}