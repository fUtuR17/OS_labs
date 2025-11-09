#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
    int *arr;
    int n;
    int num_threads;
    int thread_id;
    pthread_barrier_t *barrier;
} ThreadData;

// Сравнение и обмен
void compare_and_swap(int *arr, int i, int j, int ascending) {
    if (ascending == (arr[i] > arr[j])) {
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

// Основная функция, выполняемая каждым потоком
void *bitonic_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int *arr = data->arr;
    int n = data->n;
    int tid = data->thread_id;
    int num_threads = data->num_threads;


    for (int k = 2; k <= n; k <<= 1) {
        for (int j = k >> 1; j > 0; j >>= 1) {

            // Каждый поток обрабатывает свои индексы
            for (int i = tid; i < n; i += num_threads) {
                int ixj = i ^ j; // XOR вычисляет пару для сравнения
                // swap происходит только в том случае, если ixj > i
                if (ixj > i) {
                    int ascending = ((i & k) == 0); // направление сортировки
                    compare_and_swap(arr, i, ixj, ascending);
                }
            }

            // Синхронизация между потоками
            pthread_barrier_wait(data->barrier);
        }
    }

    pthread_exit(NULL);
}

int is_power_of_two(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

void print_array(int *arr, int n) {
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

int main() {
    int n, num_threads;
    printf("Введите длину массива (степень двойки): ");
    scanf("%d", &n);

    if (!is_power_of_two(n)) {
        printf("Ошибка: длина массива должна быть степенью двойки!\n");
        return 1;
    }

    printf("Введите количество потоков (1–8): ");
    scanf("%d", &num_threads);
    if (num_threads < 1 || num_threads > 8) {
        printf("Ошибка: число потоков должно быть от 1 до 8.\n");
        return 1;
    }

    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        perror("malloc");
        return 1;
    }

    int mode;
    printf("Режим ввода: 1 — вручную, 2 — случайно: ");
    scanf("%d", &mode);

    if (mode == 1) {
        printf("Введите %d элементов массива:\n", n);
        for (int i = 0; i < n; i++) scanf("%d", &arr[i]);
    } else {
        srand(time(NULL));
        for (int i = 0; i < n; i++) arr[i] = rand() % 1000;
        printf("Сгенерированный массив:\n");
        print_array(arr, n);
    }

    pthread_t threads[num_threads];
    ThreadData data[num_threads];
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Создание потоков
    for (int t = 0; t < num_threads; t++) {
        data[t].arr = arr;
        data[t].n = n;
        data[t].num_threads = num_threads;
        data[t].thread_id = t;
        data[t].barrier = &barrier;
        pthread_create(&threads[t], NULL, bitonic_thread, &data[t]);
    }

    // Ожидание завершения
    for (int t = 0; t < num_threads; t++) pthread_join(threads[t], NULL);

    gettimeofday(&end, NULL);
    pthread_barrier_destroy(&barrier);

    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
                     (end.tv_usec - start.tv_usec) / 1000.0;

    printf("\nОтсортированный массив:\n");
    print_array(arr, n);
    printf("\nВремя сортировки: %.3f мс (%d потоков)\n", elapsed, num_threads);

    free(arr);
    return 0;
}
