#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_BUFFER_SIZE 4096

extern char **environ;

char *custom_getline(void)
{
    static char buffer[MAX_BUFFER_SIZE];
    static size_t buffer_index = 0;
    static ssize_t bytes_read = 0;
    char *line = buffer + buffer_index;
    if (buffer_index >= (size_t)bytes_read || bytes_read <= 0)
    {
        bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes_read <= 0)
        {
            return NULL;
        }

        buffer_index = 0;
    }

    buffer_index += strcspn(line, "\n") + 1;

    line[strcspn(line, "\n")] = '\0';

    return line;
}

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
    char *full_path = NULL;

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
    char *command_path = find_command(args[0]);
    pid_t pid;
    int status;
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
    char *input;
    char *token;
    char *args[MAX_INPUT_SIZE];
    int arg_count;

    while (1)
    {
        input = custom_getline();
        if (input == NULL)
        {
            printf("\n");
            break;
        }

        arg_count = 0;

        display_prompt();

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
 
