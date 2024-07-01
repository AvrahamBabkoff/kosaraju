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
#include "proactor.h"

#define POLL_TIMEOUT_INFINITE -1

struct proactor
{
    proactorFunc func;
    int pipe_fds[2];
    int sockfd;
};

struct proactor_client
{
    // identifier of this client
    int id;
    // client socket
    int socketfd;
    // method to invoke with socketfd
    proactorFunc func;
    // writing end of a pipe to notify client disconnection
    int pipe_write;
};

void *invokeProactorFunc(void *arg)
{
    struct proactor_client* prct_client = (struct proactor_client*)arg;
    prct_client->func(prct_client->socketfd);
    // if here, we signal the main proactor thread that we are terminating
    write(prct_client->pipe_write, &prct_client->id, sizeof(prct_client->id));
}

void *proactorMainThread(void *arg)
{
    struct proactor *prct = (struct proactor *)arg;
    (void)prct;
    struct pollfd fds[2];

    // Set up poll structure

    // add listner
    fds[0].fd = prct->sockfd;
    fds[0].events = POLLIN;

    // add read end of pipe
    fds[1].fd = prct->pipe_fds[0];
    fds[1].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, 2, POLL_TIMEOUT_INFINITE);

        if (ret == -1)
        {
            perror("poll");
            break;
        }
        if (fds[0].revents & POLLIN)
        {
            // Accept new connection
            int new_socket = accept(prct->sockfd, NULL, NULL);
            if (new_socket == -1)
            {
                perror("accept");
            }
            else
            {
                pthread_t thread;
                if (pthread_create(&thread, NULL, invokeProactorFunc, (void *)(long)client) != 0)
                {
                    perror("pthread_create");
                    break;
                }
            }
        }
    }
    return NULL;
}
void *startProactor(int sockfd, proactorFunc threadFunc)
{
    struct proactor *prct = (struct proactor *)malloc(sizeof(struct proactor));
    prct->func = threadFunc;
    prct->sockfd = sockfd;
    if (pipe(prct->pipe_fds) == -1)
    {
        perror("pipe");
        free(prct);
        prct = NULL;
    }
    else
    {
        pthread_t thread;
        if (pthread_create(&thread, NULL, proactorMainThread, NULL) != 0)
        {
            perror("pthread_create");
            close(prct->pipe_fds[0]);
            close(prct->pipe_fds[1]);
            free(prct);
            prct = NULL;
        }
    }

    return prct;
}