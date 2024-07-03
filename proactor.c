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
#define INITIAL_NUM_OF_CLIENTS 10

struct proactor
{
    proactorFunc func;
    int pipe_fds[2];
    int sockfd;
    pthread_t thread_id;
};

struct proactor_client
{
    // client socket
    int socketfd;
    // method to invoke with socketfd
    proactorFunc func;
    // writing end of a pipe to notify client disconnection
    int pipe_write;
};

struct proactor_client_thread
{
    struct proactor_client client;
    pthread_t thread;
};

void *invokeProactorFunc(void *arg)
{
    struct proactor_client *prct_client = (struct proactor_client *)arg;
    prct_client->func(prct_client->socketfd);
    // if here, we signal the main proactor thread that we are terminating
    write(prct_client->pipe_write, &prct_client->socketfd, sizeof(prct_client->socketfd));
    return NULL;
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
    int clients_size = INITIAL_NUM_OF_CLIENTS;
    int clients_count = 0;
    struct proactor_client_thread *clients = (struct proactor_client_thread *)malloc(sizeof(struct proactor_client_thread) * clients_size);
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
                if (clients_size == clients_count)
                {
                    clients_size *= 2;
                    clients = (struct proactor_client_thread *)realloc(clients, sizeof(struct proactor_client_thread) * clients_size);
                }
                clients[clients_count].client.socketfd = new_socket;
                clients[clients_count].client.pipe_write = prct->pipe_fds[1];
                clients[clients_count].client.func = prct->func;
                if (pthread_create(&thread, NULL, invokeProactorFunc, &(clients[clients_count].client)) != 0)
                {
                    perror("pthread_create");
                    break;
                }
                clients[clients_count].thread = thread;
                ++clients_count;
            }
        }
        else if (fds[1].revents & POLLIN)
        {
            int fd;
            read(prct->pipe_fds[0], &fd, sizeof(fd));
            if (fd >= 0)
            {
                // signalled by client, find client slot
                for (int i = 0; i < clients_count; i++)
                {
                    if (fd == clients[i].client.socketfd)
                    {
                        clients[i] = clients[clients_count - 1];
                        --clients_count;
                        close(fd);
                    }
                }
            }
            else
            {
                // we were signalled to shutdown, go over all clients and shutdown the sockets
                for (int i = 0; i < clients_count; i++)
                {
                    shutdown(clients[i].client.socketfd, SHUT_RDWR);
                }
                for (int i = 0; i < clients_count; i++)
                {
                    struct timespec ts;
                    int s;

                    clock_gettime(CLOCK_REALTIME, &ts);

                    ts.tv_sec += 2;

                    s = pthread_timedjoin_np(clients[i].thread, NULL, &ts);
                    if (s != 0)
                    {
                        printf("cancelling thread %ld", clients[i].thread);
                        pthread_cancel(clients[i].thread);
                    }
                }
                break;
            }
        }
    }
    free(clients);
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
        if (pthread_create(&prct->thread_id, NULL, proactorMainThread, prct) != 0)
        {
            perror("pthread_create");
            close(prct->pipe_fds[0]);
            close(prct->pipe_fds[1]);
            free(prct);
            prct = NULL;
        }
        else
        {
            printf("created proactor thread %ld\n", prct->thread_id);
        }
    }

    return prct;
}

int stopProactor(void *proactor)
{
    struct proactor *prct = (struct proactor *)proactor;
    pthread_t thread_id = prct->thread_id;
    int fd = -1;
    write(prct->pipe_fds[1], &fd, sizeof(fd));
    // wait for thread to terminate
    struct timespec ts;
    int s;

    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += 2;

    s = pthread_timedjoin_np(thread_id, NULL, &ts);
    if (s != 0)
    {
        printf("cancelling proactor thread %ld\n", thread_id);
        pthread_cancel(thread_id);
    }
    else
    {
        printf("proactor thread %ld terminated\n", thread_id);
    }
    return 0;
}