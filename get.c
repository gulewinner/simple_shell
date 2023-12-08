#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024

void display_prompt(void)
{
    printf("#cisfun$ ");
    fflush(stdout);
}

int main(void)
{
    char input[MAX_INPUT_SIZE];
    pid_t pid;
    int status;

    while (1)
    {
        display_prompt();

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        pid = fork();

        if (pid == -1)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            if (execlp(input, input, (char *)NULL) == -1)
            {
                fprintf(stderr, "./shell: %s: No such file or directory\n", input);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (waitpid(pid, &status, 0) == -1)
            {
                perror("Wait failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    printf("Exiting shell\n");
    return 0;
}

