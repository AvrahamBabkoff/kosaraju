#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "listner.h"

int createListner(const char *port)
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    // addFdToReactor(reactor, listener, clientConnected);
    return listener;
}
