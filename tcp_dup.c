#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tcp_dup.h"

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

