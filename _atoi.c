#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(void) {
    char *prompt = "#cisfun$ ";
    char command[BUFFER_SIZE];

    while (1) {
        printf("%s", prompt);

        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            printf("\n");
            break;
        }

        
        command[strcspn(command, "\n")] = '\0';

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
           
            if (execlp(command, command, NULL) == -1) {
                perror("execlp");
                exit(EXIT_FAILURE);
            }

            
            exit(EXIT_FAILURE);
        } else {
            
            int status;
            if (wait(&status) == -1) {
                perror("wait");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) {
             
                int exit_status = WEXITSTATUS(status);

                if (exit_status != 0) {
                    printf("%s: command not found\n", command);
                }
            } else if (WIFSIGNALED(status)) {
                
                int signal_number = WTERMSIG(status);
                printf("%s: terminated by signal %d\n", command, signal_number);
            }
        }
    }

    return EXIT_SUCCESS;
}
