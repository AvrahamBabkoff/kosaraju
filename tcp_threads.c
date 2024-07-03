#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tcp_dup.h"
#include "kosaraju.h"
#include "listner.h"
#include "proactor.h"

pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

/************************************************************
* processClientFd:
* 
* Implementation of the dedicated thread per client function 
* of stage 7 and proactor function of stage 9.
************************************************************/

/**
* enter infinite loop blocking on recv. 
* thread starts for a new connection, when recv returns, a nre thread 
* is created with processClient as the thread function, the client socket
* is passed as a thread argument
*/

void *processClientFd(int client)
{
    char buf[256]; // Buffer for client data
    pthread_mutex_lock(&_mutex);

    printCommandsToFd(client);

    pthread_mutex_unlock(&_mutex);
    while (1)
    {
        int nbytes = recv(client, buf, sizeof(buf) - 1, 0);
        if (nbytes <= 0)
        {
            // Got error or connection closed by client
            if (nbytes == 0)
            {
                // Connection closed
                printf("processClient: socket %d hung up\n", client);
            }
            else
            {
                perror("recv");
            }

            close(client); // Bye!
            break;
        }
        else
        {
            buf[nbytes] = '\0';
            pthread_mutex_lock(&_mutex);
            executeCommandToFd(client, buf);
            pthread_mutex_unlock(&_mutex);
        }
    }
    return NULL;
}


void *processClient(void *arg)
{
    int client = (long)arg;
    return processClientFd(client);
}


/************************************************************
* acceptAndCreateThreadPerClients:
* 
* Implementation of the main function of stage 7.
************************************************************/

/**
* enter infinite loop blocking on accept. when accept returns, a new thread 
* is created with processClient as the thread function, the client socket
* is passed as a thread argument
*/

void acceptAndCreateThreadPerClients(const char *port)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    int listner = createListner(port);

    while (1)
    {
        int client;
        if ((client = accept(listner, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            close(listner);
            break;
        }
        else
        {
            pthread_t thread;
            if (pthread_create(&thread, NULL, processClient, (void *)(long)client) != 0)
            {
                perror("pthread_create");
                break;
            }
        }
    }
}

void *createAndAddListnerToProactor(const char *port)
{
    int listner = createListner(port);
    void *proactor = startProactor(listner, *processClientFd);
    return proactor;
}

void shutdownProactor(void *proactor)
{
    stopProactor(proactor);
}