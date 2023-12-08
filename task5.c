#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_PATH_SIZE 1024

extern char **environ;

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

    while (token != NULL)
    {
        char *full_path = malloc(strlen(token) + strlen(command) + 2);
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
    char *command_path;

    command_path = find_command(args[0]);

    if (strcmp(args[0], "env") == 0)
    {
        char **env = environ;
        while (*env)
        {
            printf("%s\n", *env++);
        }
        return 0;
    }

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

        if (strcmp(args[0], "exit") == 0)
        {
            printf("Exiting shell\n");
            break;
        }

        execute_command(args);
    }

    return 0;
}

