#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_PATH_SIZE 1024

void display_prompt(void)
{
    printf(":) ");
    fflush(stdout);
}

char *get_path(void)
{
    char *path = getenv("PATH");
    if (path == NULL)
    {
        perror("Error getting PATH");
        exit(EXIT_FAILURE);
    }
    return path;
}

char *find_command(char *command)
{
    char *path = get_path();
    char *token = strtok(path, ":");
    char *full_path;

    while (token != NULL)
    {
        full_path = malloc(strlen(token) + strlen(command) + 2);
        if (full_path == NULL)
        {
            perror("Malloc error");
            exit(EXIT_FAILURE);
        }

        sprintf(full_path, "%s/%s", token, command);

        if (access(full_path, F_OK) == 0)
        {
            return full_path;
        }

        free(full_path);
        token = strtok(NULL, ":");
    }

    return NULL;
}

int execute_command(char *args[])
{
    pid_t pid;
    int status;

    char *command_path = find_command(args[0]);

    if (command_path == NULL)
    {
        fprintf(stderr, "Command not found: %s\n", args[0]);
        return 1;
    }

    pid = fork();

    if (pid == -1)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        if (execve(command_path, args, NULL) == -1)
        {
            perror("Execve failed");
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

    free(command_path);
    return 0;
}

int main(void)
{
    char input[MAX_INPUT_SIZE];
    char *token;
    char *args[MAX_INPUT_SIZE];
    int arg_count = 0;

    while (1)
    {
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

    printf("\nExiting shell\n");
    return 0;
}

