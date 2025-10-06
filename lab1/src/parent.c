  #include <string.h>
  #include <stdio.h>
  #include <unistd.h> // Системные вызовы
  #include <stdlib.h> // Выход из программмы через exit, дин. пам. тут нет
  #include <sys/wait.h> // Ожидание завершения дочернего процесса


  #define MAX_CHAR 256
  #define READ_END 0
  #define WRITE_END 1

  int main(){
      // Инициализация и считывание названий файлов
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
      // Обработка названий файлов
      filename1[strcspn(filename1, "\n")] = '\0';
      filename2[strcspn(filename2, "\n")] = '\0';

      
      int pipe1[2], pipe2[2];
      pid_t pid1, pid2;

      // Инициализация файловых дескрипторов для кажого из пайпов
      if (pipe(pipe1) == -1 || pipe(pipe2) == -1){
        perror("descriptors initialization error: pipe error");
        exit(1);
      }

      // ----------------------
      // 1 форк
      // ---------------------- 

      switch(pid1 = fork()){

      // Fork не удался
        case -1:
          perror("fork 1 error");
          exit(1);
        case 0:
          close(pipe1[WRITE_END]);
          close(pipe2[WRITE_END]);
          close(pipe2[READ_END]);
        // Переназначение 0 дескриптора на pipe2[READ_END]
          if (dup2(pipe1[READ_END], STDIN_FILENO) == -1){
            perror("dup error");
            exit(1);
          }
        
          // Надо закрывать не только запись, но и чтение, потому что 
          // чтение и так будет обеспечено из 0 канала, переназначенного на pipe1[READ_END]
          // и не нужно держать еще один дескриптор, ссылающийся на pipe1[READ_END], так как ядро будет думать, 
          // что из него тоже может прийти какая-то информация, раз он не закрыт, и не будет закрывать процесссы, 
          // завершающиеся через EOF, например getline.
          
          // Тогда, при закрытии pipe1[WRITE_END] у родителя будет автоматически отправлен EOF и функции завершатся
          
          close(pipe1[READ_END]);
        
          execl("./child1", "./child1", filename1, NULL);
          
          // Выполнится, если execl не отработает как надо
          perror("execl child 1 error");
          exit(1);

        default:
          // ----------------------
          // 2 форк
          // ----------------------
          
          switch(pid2 = fork()){

          case -1:
            perror("fork 1 error");
            exit(1);
          case 0:
            close(pipe1[READ_END]);
            close(pipe1[WRITE_END]);
            close(pipe2[WRITE_END]);
            
            // Переназначение 0 дескриптора на pipe2[READ_END]
            if (dup2(pipe2[READ_END], STDIN_FILENO) == -1){
              perror("dup error");
              exit(1);
            }
            
            close(pipe2[READ_END]);
            execl("./child2", "./child2", filename2, NULL);

            // Выполнится, если execl не отработает как надо
            perror("execl child 2 error");
            exit(1);

          default:
            close(pipe1[READ_END]);
            close(pipe2[READ_END]);

            // ----------------------
            // Считывание строк
            // ----------------------

            char *buffer = NULL;
            size_t size = 0;
            
            while (1) {
                printf("Введите строку или 'exit' для выхода:\n");
                fflush(stdout);
                ssize_t len = getline(&buffer, &size, stdin);
                if (len == -1) break;   // EOF или ошибка

                // buffer[strcspn(buffer, "\n")] = '\0';  // Если доавлена, то тогда 10, а не 11

                if (strcmp(buffer, "exit\n") == 0){
                  close(pipe1[WRITE_END]);
                  close(pipe2[WRITE_END]);
                  break;
                } 

                if (strlen(buffer) <= 10 + 1) {
                    if (write(pipe1[WRITE_END], buffer, len) == -1){
                      perror("writing to pipe1 error");
                      exit(1);
                    }
                } else {
                    if (write(pipe2[WRITE_END], buffer, len) == -1){
                      perror("writing to pipe2 error");
                      exit(1);
                    }
                }

                // printf("%s", buffer);
            }

            // ----------------------
            // Завершение
            // ----------------------

            free(buffer);
            wait(NULL);
            wait(NULL);
            return 0;
        }
      }
    }
