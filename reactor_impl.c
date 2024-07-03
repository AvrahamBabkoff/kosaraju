
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "reactor_impl.h"
#include "kosaraju.h"
#include "tcp_dup.h"
#include "listner.h"

void clientData(int fd, void *reactor)
{
    char buf[256]; // Buffer for client data
    int nbytes = recv(fd, buf, sizeof(buf) - 1, 0);
    if (nbytes <= 0)
    {
        // Got error or connection closed by client
        if (nbytes == 0)
        {
            // Connection closed
            printf("clientData: socket %d hung up\n", fd);
        }
        else
        {
            perror("recv");
        }
        close(fd);
        removeFdFromReactor(reactor, fd);
    }
    else
    {
        buf[nbytes] = '\0';
        executeCommandToFd(fd, buf);
    }
}

void clientConnected(int fd, void *reactor)
{
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    addrlen = sizeof remoteaddr;
    int newfd = accept(fd,
                       (struct sockaddr *)&remoteaddr,
                       &addrlen);
    if (newfd != -1)
    {
        printCommandsToFd(newfd);
        addFdToReactor(reactor, newfd, clientData);
    }
}

void createAndAddListnerToReactor(const char *port, void *reactor)
{
    int listner = createListner(port);
    addFdToReactor(reactor, listner, clientConnected);
}