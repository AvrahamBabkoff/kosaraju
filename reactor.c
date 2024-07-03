
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
#include <pthread.h>
#include "reactor.h"

#define INITIAL_NUM_OF_FDS 10
struct ReactorHandler
{
    int fd;
    reactorFunc func;
};

struct reactor
{
    struct ReactorHandler *reactorHandlers;
    int fd_size;
    int fd_count;
    bool running;
    // used to signal shutdown
    int pipe_fds[2];
};

void shutdownFunc(int fd, void *reactor)
{
    (void)fd;
    printf("shutting down reactor\n");
    struct reactor *rct = (struct reactor *)reactor;
    rct->running = false;
}

void *createtReactor()
{
    struct reactor *newReactor = (struct reactor *)malloc(sizeof(struct reactor));
    if (pipe(newReactor->pipe_fds) == -1)
    {
        perror("pipe");
        free(newReactor);
        newReactor = NULL;
    }
    else
    {
        newReactor->reactorHandlers = (struct ReactorHandler *)malloc(sizeof(struct ReactorHandler) * INITIAL_NUM_OF_FDS);
        newReactor->fd_count = 0;
        newReactor->fd_size = INITIAL_NUM_OF_FDS;
        newReactor->running = false;
        addFdToReactor(newReactor, newReactor->pipe_fds[0], shutdownFunc);
    }

    return newReactor;
}

int addFdToReactor(void *reactor_instance, int fd, reactorFunc func)
{
    // chek size and count
    struct reactor *rct = (struct reactor *)reactor_instance;
    if (rct->fd_size == rct->fd_count)
    {
        // double the size
        rct->fd_size *= 2;
        rct->reactorHandlers = (struct ReactorHandler *)realloc(rct->reactorHandlers, sizeof(struct ReactorHandler) * rct->fd_size);
        printf("reallocated reactor to size %d, count will be %d", rct->fd_size, rct->fd_count + 1);
    }
    rct->reactorHandlers[rct->fd_count].fd = fd;
    rct->reactorHandlers[rct->fd_count].func = func;
    rct->fd_count++;
    printf("added new func to reactor size %d, count  %d\n", rct->fd_size, rct->fd_count);

    return 1;
}

int removeFdFromReactor(void *reactor_instance, int fd)
{
    // check size and count
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

void fillPollArray(struct reactor *rct, struct pollfd *fds)
{
    for (int i = 0; i < rct->fd_count; i++)
    {
        fds[i].fd = rct->reactorHandlers[i].fd;
        fds[i].events = POLLIN;
    }
}

void *reactorMainThread(void *arg)
{
    struct reactor *rct = (struct reactor *)arg;

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
                    rct->reactorHandlers[i].func(rct->reactorHandlers[i].fd, rct);
                }
            }
        }
    }
    // if here, we were signalled to exit
    printf("shutting down reactor thread func\n");
    for (int i = 0; i < rct->fd_count; i++)
    {
        close(rct->reactorHandlers[i].fd);
        close(rct->pipe_fds[1]);
    }
    free(rct->reactorHandlers);
    free(rct);
    return NULL;
}

int startReactor(void *reactor_instance)
{
    int retVal = 0;
    struct reactor *rct = (struct reactor *)reactor_instance;
    pthread_t thread;
    if (pthread_create(&thread, NULL, reactorMainThread, rct) != 0)
    {
        perror("pthread_create");
        retVal = -1;
    }

    return retVal;
}

int stopReactor(void *reactor_instance)
{
    struct reactor *rct = (struct reactor *)reactor_instance;
    char ch = 0;
    write(rct->pipe_fds[1], &ch, sizeof(ch));
    // reactor_instance is no longer valid
    return  0;
}