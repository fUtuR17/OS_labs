#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "calc.h"

int main(void) {
    char cmd;

    while (1) {
        if (scanf(" %c", &cmd) != 1) {
            break;
        }

        if (cmd == 'q' || cmd == 'Q') {
            break;
        } else if (cmd == '1') {
            float A, B;
            if (scanf("%f %f", &A, &B) != 2) {
                return 1;
            }
            float g = Square(A, B);
            printf("%f\n", g);
        } else if (cmd == '2') {
            int size;
            if (scanf("%d", &size) != 1 || size <= 0) {
                return 1;
            }
            
            int* array = (int*)malloc(size * sizeof(int));
            if (array == NULL) {
                return 1;
            }
            
            for (int i = 0; i < size; i++) {
                if (scanf("%d", &array[i]) != 1) {
                    free(array);
                    return 1;
                }
            }
            
            int* sorted = Sort(array, size);
            
            for (int i = 0; i < size; i++) {
                printf("%d", sorted[i]);
                if (i < size - 1) {
                    printf(" ");
                }
            }
            printf("\n");
            
            free(array);
        } else {
        }
    }
    return 0;
}