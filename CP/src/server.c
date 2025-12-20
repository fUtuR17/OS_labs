#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "common.h"

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open");
        return 1;
    }

    ftruncate(fd, sizeof(SharedData));

    SharedData *data = mmap(NULL, sizeof(SharedData),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    memset(data, 0, sizeof(SharedData));
    sem_init(&data->mutex, 1, 1);

    printf("=== Chat server started ===\n");

    while (1) {
        sleep(1);
    }
}
