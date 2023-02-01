#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define EXIT_SUCCESS 0

int numOfArgs = 0, pipeExist = 0, numOfPipes = -1;

char **tokenize(char *line)
{
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    int i, tIndex = 0, tNumber = 0;

    for (i = 0; i < strlen(line); i++)
    {
        char readChar = line[i];

        if (readChar == '|')
            pipeExist = 1;

        if (readChar == ' ' || readChar == '\n' || readChar == '\t')
        {
            token[tIndex] = '\0';
            if (tIndex != 0)
            {
                tokens[tNumber] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tNumber++], token);
                tIndex = 0;
            }
        }
        else
            token[tIndex++] = readChar;
    }
    free(token);
    tokens[tNumber] = NULL;
    numOfArgs = tNumber;
    return tokens;
}

char **create_pipe(char *line)
{
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    int i, tIndex = 0, tNumber = 0;

    for (i = 0; i < strlen(line); i++)
    {
        char readChar = line[i];
        if (readChar == '|' || readChar == '\n')
        {
            numOfPipes++;
            token[tIndex] = '\0';
            if (tIndex != 0)
            {
                tokens[tNumber] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tNumber++], token);
                tIndex = 0;
            }
        }
        else
            token[tIndex++] = readChar;
    }

    free(token);
    tokens[tNumber] = NULL;
    return tokens;
}

int main(int argc, char *argv[])
{
    int i;
    char line[MAX_INPUT_SIZE];
    char **tokens;
    FILE *fp;

    if (argc == 2)
    {
        fp = fopen(argv[1], "r");
        if (fp < 0)
        {
            printf("Sorry, File does not Exist.\n");
            return -1;
        }
    }

    while (1)
    {
        memset(line, 0, sizeof(line));

        if (argc == 2)
        {
            if (fgets(line, sizeof(line), fp) == NULL)
                break;
            line[strlen(line) - 1] = '\0';
        }
        else
        {
            char cwd[256];

            printf("[" COLOR_GREEN "MyShell:" COLOR_RESET);
            printf("%s%s%s]", COLOR_MAGENTA, getcwd(cwd, sizeof(cwd)), COLOR_RESET);
            printf("$ ");
            scanf("%[^\n]", line);
            getchar();
        }

        line[strlen(line)] = '\n';
        tokens = tokenize(line);
        if (pipeExist == 1)
        {
            int *pipes = (int *)malloc(2 * numOfPipes * sizeof(int));
            char **pipe_cmd = create_pipe(line);
            int i, j, stat;

            for (i = 0; i < numOfPipes; i++)
                pipe(pipes + 2 * i);
            for (i = 0; i <= numOfPipes; i++)
            {
                pid_t pid = fork();
                if (pid < 0)
                {
                    printf("Sorry, Can't be forked.\n");
                    continue;
                }
                if (pid == 0)
                {
                    if (i != 0)
                        dup2(pipes[2 * (i - 1)], 0);
                    if (i != numOfPipes)
                        dup2(pipes[2 * i + 1], 1);
                    for (j = 0; j < numOfPipes; j++)
                    {
                        close(pipes[2 * j]);
                        close(pipes[2 * j + 1]);
                    }
                    pipe_cmd[i][strlen(pipe_cmd[i])] = '\n';
                    tokens = tokenize(pipe_cmd[i]);
                    execvp(tokens[0], tokens);
                    printf("Sorry, you've entered an invalid command.\n");
                }
            }

            for (j = 0; j < numOfPipes; j++)
            {
                close(pipes[2 * j]);
                close(pipes[2 * j + 1]);
            }
            for (i = 0; i <= numOfPipes; i++)
                wait(&stat);
            for (i = 0; pipe_cmd[i] != NULL; i++)
            {
                free(pipe_cmd[i]);
            }
            free(pipes);
            pipeExist = 0;
            numOfPipes = -1;
        }
        else
        {
            tokens = tokenize(line);
            int background = 0;
            if (strcmp(tokens[numOfArgs - 1], "&") == 0)
            {
                background = 1;
                tokens[numOfArgs - 1] = '\0';
            }
            if (strcmp(tokens[0], "exit") == 0)
            {
                exit(0);
            }

            else if (strcmp(tokens[0], "cd") == 0)
            {
                chdir(tokens[1]);
            }
            else if (strcmp(tokens[0], "kill") == 0)
            {
                int stat;
                kill((pid_t)atoi(tokens[1]), SIGUSR1);
                wait(&stat);
            }
            else
            {
                pid_t child_pid = fork();
                if (child_pid < 0)
                {
                    printf("Sorry, Can't be forked.\n");
                    continue;
                }
                if (child_pid == 0)
                {
                    execvp(tokens[0], tokens);
                    printf("You've entered an invalid command.\n");
                    exit(0);
                }
                else if (background != 1)
                    waitpid(child_pid, NULL, 0);
            }
        }

        for (i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }

    return EXIT_SUCCESS;
}