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

int execute_command(char *args[])
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid == -1)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            fprintf(stderr, "./shell: %s: No such file or directory\n", args[0]);
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

    return 0;
}

int main(void)
{
    char input[MAX_INPUT_SIZE];

    while (1)
    {
        char *token;
        char *args[MAX_INPUT_SIZE];
        int arg_count = 0;

        display_prompt();

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        token = strtok(input, " ");
        while (token != NULL)
        {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }

        args[arg_count] = NULL;

        execute_command(args);
    }

    printf("Exiting shell\n");
    return 0;
}
	
