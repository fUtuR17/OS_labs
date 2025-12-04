#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        perror("Not enough arguments\n");
        exit(1);
    }

    FILE *f = fopen(argv[1], "w");
    if (!f) {
        perror("fopen error");
        exit(1);
    }

    // Открываем shared memory
    int shm_fd = shm_open(argv[2], O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open error in child");
        exit(1);
    }

    // Отображаем shared memory
    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), 
                                     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap error in child");
        exit(1);
    }

    while (1) {
        int processed_something = 0;
        
        // Проверяем все строки
        for (int i = 0; i < shared_data->count; i++) {
            // Обрабатываем только короткие строки (<= 11 символов) и которые еще не обработаны
            if (shared_data->lengths[i] <= 11 && !shared_data->processed_by_child1[i]) {
                // Реверсируем строку
                for (int j = shared_data->lengths[i] - 2; j >= 0; j--) {
                    fputc(shared_data->data[i][j], f);
                }
                fputc('\n', f);
                fflush(f);
                
                // Помечаем как обработанную
                shared_data->processed_by_child1[i] = 1;
                processed_something = 1;
            }
        }

        // Если все завершено и больше нечего обрабатывать - выходим
        if (shared_data->finished) {
            int all_processed = 1;
            for (int i = 0; i < shared_data->count; i++) {
                if (shared_data->lengths[i] <= 11 && !shared_data->processed_by_child1[i]) {
                    all_processed = 0;
                    break;
                }
            }
            if (all_processed) break;
        }

        // Если ничего не обработали в этой итерации - небольшая пауза
        if (!processed_something) {
            usleep(10000); // 10ms
        }
    }

    fclose(f);
    munmap(shared_data, sizeof(shared_data_t));
    return 0;
}