
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include "poll_reactor.h"

#define INITIAL_NUM_OF_FDS 10
struct ReactorHandler
{
    int fd;
    reactorFunc func;
};

struct reactor
{
    struct ReactorHandler *reactorHandlers;
    // struct pollfd *pfds;
    int fd_size;
    int fd_count;
    bool running;
};

void *createtReactor()
{
    printf("Hello again from reactor library!!!\n");
    struct reactor *newReactor = (reactor *)malloc(sizeof(reactor));
    // newReactor->pfds = (struct pollfd *)malloc(sizeof(struct pollfd) * INITIAL_NUM_OF_FDS);
    newReactor->reactorHandlers = (struct ReactorHandler *)malloc(sizeof(struct ReactorHandler) * INITIAL_NUM_OF_FDS);
    newReactor->fd_count = 0;
    newReactor->fd_size = INITIAL_NUM_OF_FDS;
    newReactor->running = false;

    return newReactor;
}

int addFdToReactor(void *reactor_instance, int fd, reactorFunc func)
{
    // chek size and count
    struct reactor *rct = (struct reactor *)reactor_instance;
    if (rct->fd_size == rct->fd_count)
    {
        // double size
        rct->fd_size *= 2;
        // rct->pfds = (struct pollfd *)realloc(rct->pfds, sizeof(struct pollfd) * rct->fd_size);
        rct->reactorHandlers = (struct ReactorHandler *)realloc(rct->reactorHandlers, sizeof(struct ReactorHandler) * rct->fd_size);
        printf("reallocated reactor to size %d, count will be %d", rct->fd_size, rct->fd_count + 1);
    }
    // rct->pfds[rct->fd_count].fd = fd;
    // rct->pfds[rct->fd_count].events = POLLIN;
    rct->reactorHandlers[rct->fd_count].fd = fd;
    rct->reactorHandlers[rct->fd_count].func = func;
    rct->fd_count++;
    printf("added new func to reactor size %d, count  %d\n", rct->fd_size, rct->fd_count);

    return 1;
}

int removeFdFromReactor(void *reactor_instance, int fd)
{
    // chek size and count
    struct reactor *rct = (struct reactor *)reactor_instance;
    int removed = 0;
    for (int i = 0; i < rct->fd_count; i++)
    {
        if (rct->reactorHandlers[i].fd == fd)
        {
            printf("removing %d ", fd);
            // rct->pfds[i] = rct->pfds[rct->fd_count - 1];
            rct->reactorHandlers[i] = rct->reactorHandlers[rct->fd_count - 1];
            rct->fd_count--;
            printf("size %d, count %d\n", rct->fd_size, rct->fd_count);

            removed = 1;
            break;
        }
    }
    return removed;
}

// void printReactor(void *reactor_instance)
// {
//     // chek size and count
//     struct reactor *rct = (struct reactor *)reactor_instance;
//     for (int i = 0; i < rct->fd_count; i++)
//     {
//         printf("fd: %d, ", rct->reactorHandlers[i].socket);
//         rct->reactorHandlers[i].func( rct->reactorHandlers[i].socket, reactor_instance);
//         printf("\n");
//     }
// }

void fillPollArray(struct reactor *rct, struct pollfd *fds)
{
    for (int i = 0; i < rct->fd_count; i++)
    {
        fds[i].fd = rct->reactorHandlers[i].fd;
        fds[i].events = POLLIN;
    }
}

int startReactor(void *reactor_instance)
{
    struct reactor *rct = (struct reactor *)reactor_instance;

    rct->running = true;
    while (rct->running)
    {
        struct pollfd fds[rct->fd_count];

        // fds correscponds in size to rct->reactorHandlers
        fillPollArray(rct, fds);
        int poll_count = poll(fds, rct->fd_count, 1000);
        if (poll_count == -1)
        {
            rct->running = false;
            perror("poll");
        }
        else if (poll_count > 0)
        {
            for (int i = 0; i < rct->fd_count; i++)
            {
                if (fds[i].revents & POLLIN)
                {
                    rct->reactorHandlers[i].func(rct->reactorHandlers[i].fd, reactor_instance);
                }
            }
        }
    }
    return 1;
}