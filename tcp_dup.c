#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tcp_dup.h"
#include "kosaraju.h"

int tcp_dup_std(int fd, int std)
{
    int saved_stdout = dup(std);
    if (saved_stdout < 0)
    {
        perror("dup");
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, std) < 0)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    return saved_stdout;
}

void tcp_undup_std(int saved_stdout, int std)
{
    if (dup2(saved_stdout, std) < 0)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

void printCommandsToFd(int fd)
{
    int saved_stdout = tcp_dup_std(fd, STDOUT_FILENO);
    int saved_stdin = tcp_dup_std(fd, STDIN_FILENO);
    printCommands();
    tcp_undup_std(saved_stdout, STDOUT_FILENO);
    tcp_undup_std(saved_stdin, STDIN_FILENO);
}

void executeCommandToFd(int fd, char *command)
{
    int saved_stdout = tcp_dup_std(fd, STDOUT_FILENO);
    int saved_stdin = tcp_dup_std(fd, STDIN_FILENO);
    executeCommand(command);
    tcp_undup_std(saved_stdout, STDOUT_FILENO);
    tcp_undup_std(saved_stdin, STDIN_FILENO);
}
