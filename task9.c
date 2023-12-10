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
    size_t command_len = strlen(command);
    size_t path_len = strlen(path);

    char *full_path = malloc(path_len + command_len + 2);
    if (full_path == NULL)
    {
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }

    strcpy(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, command);

    if (access(full_path, F_OK) == 0)
    {
        return full_path;
    }

    free(full_path);
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

int shell_exit(char *arg)
{
    int status = 0;

    if (arg != NULL)
    {
        status = atoi(arg);
    }

    exit(status);
}

int shell_setenv(char *variable, char *value)
{
    if (variable == NULL || value == NULL)
    {
        fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
        return 1;
    }

    if (setenv(variable, value, 1) == -1)
    {
        perror("setenv failed");
        return 1;
    }

    return 0;
}

int shell_unsetenv(char *variable)
{
    if (variable == NULL)
    {
        fprintf(stderr, "Usage: unsetenv VARIABLE\n");
        return 1;
    }

    if (unsetenv(variable) == -1)
    {
        perror("unsetenv failed");
        return 1;
    }

    return 0;
}

int main(void)
{
    char *input;
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

        while (*input != '\0')
        {
            while (*input == ' ' || *input == '\t' || *input == '\n')
            {
                *input++ = '\0';
            }

            args[arg_count++] = input;

            while (*input != '\0' && *input != ' ' && *input != '\t' && *input != '\n')
            {
                input++;
            }
        }

        args[arg_count] = NULL;

        if (arg_count > 0)
        {
            if (strcmp(args[0], "exit") == 0)
            {
                shell_exit(args[1]);
            }
            else if (strcmp(args[0], "setenv") == 0)
            {
                shell_setenv(args[1], args[2]);
            }
            else if (strcmp(args[0], "unsetenv") == 0)
            {
                shell_unsetenv(args[1]);
            }
            else
            {
                execute_command(args);
            }
        }
    }

    printf("\nExiting shell\n");
    return 0;
}

