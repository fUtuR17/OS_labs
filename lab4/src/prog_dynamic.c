#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "calc.h"

typedef float (*Square_fn_t)(float, float);
typedef int* (*Sort_fn_t)(int*, int);

struct Impl {
    void    *handle;
    Square_fn_t Square;
    Sort_fn_t   Sort;
    const char *path;
};

int main(void) {
    struct Impl impls[2] = {
        {NULL, NULL, NULL, "./libimpl1.so"},
        {NULL, NULL, NULL, "./libimpl2.so"}
    };

    for (int i = 0; i < 2; ++i) {
        impls[i].handle = dlopen(impls[i].path, RTLD_LAZY);
        if (!impls[i].handle) { 
            fprintf(stderr, "dlopen(%s) failed: %s\n",
                    impls[i].path, dlerror());
            for (int j = 0; j < i; ++j) {
                dlclose(impls[j].handle);
            }
            return 1;
        }

        dlerror();

        impls[i].Square = (Square_fn_t)dlsym(impls[i].handle, "Square");
        const char *err = dlerror();
        if (err) {
            fprintf(stderr, "dlsym(Square) in %s failed: %s\n",
                    impls[i].path, err);
            for (int j = 0; j <= i; ++j) {
                dlclose(impls[j].handle);
            }
            return 1;
        }

        impls[i].Sort = (Sort_fn_t)dlsym(impls[i].handle, "Sort");
        err = dlerror();
        if (err) {
            fprintf(stderr, "dlsym(Sort) in %s failed: %s\n",
                    impls[i].path, err);
            for (int j = 0; j <= i; ++j) {
                dlclose(impls[j].handle);
            }
            return 1;
        }
    }

    int current = 0;
    char cmd;

    while (1) {
    if (scanf(" %c", &cmd) != 1) {
        break;
    }

    if (cmd == 'q' || cmd == 'Q') {
        break;
    } else if (cmd == '0') {
        current = 1 - current;
    } else if (cmd == '1') {
        float A, B;
        if (scanf("%f %f", &A, &B) != 2) {
            break;
        }
        float g = impls[current].Square(A, B);
        printf("%f\n", g);
    } else if (cmd == '2') {
        int size;
        if (scanf("%d", &size) != 1 || size <= 0) {
            break;
        }
        
        int* array = (int*)malloc(size * sizeof(int));
        if (array == NULL) {
            break;
        }
        
        for (int i = 0; i < size; i++) {
            if (scanf("%d", &array[i]) != 1) {
                free(array);
                break;
            }
        }
        
        int* sorted = impls[current].Sort(array, size);
        
        for (int i = 0; i < size; i++) {
            printf("%d", sorted[i]);
            if (i < size - 1) {
                printf(" ");
            }
        }
        printf("\n");
        
        free(array);
    }
}

    for (int i = 0; i < 2; ++i) {
        if (impls[i].handle) {
            dlclose(impls[i].handle);
        }
    }

    return 0;
}