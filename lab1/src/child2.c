#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("No filenamed given\n");
        exit(1);
    }

    FILE *f = fopen(argv[1], "w");
    if (!f) {
        perror("fopen error");
        exit(1);
    }

    char *buffer = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&buffer, &len, stdin)) != -1) {
        // printf("Передана строка: %s", buffer);  // Уже есть \n
        if (read > 0){
            for (int i=read-2; i >= 0; i--){
                fputc(buffer[i], f);
            }
            fputc('\n', f);
        }
        fflush(f);
}
     
    fflush(stdout); 
    free(buffer);
    fclose(f);
    // exit(0);
    return 0;
}
