#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "kosaraju.h"
#include "pollserver.h"
#include "reactor_impl.h"
#include "tcp_threads.h"

#define PORT "9034"

void usage(void)
{
    printf("Usage: kosaraju <stage>\n"
           "    valid values of <stage>:\n"
           "        1: user enters graph vertices and edges, followed by a list of the directed edges\n"
           "        3: user can enter following commands:\n"
           "            Newgraph <verttices>,<edges>\n"
           "                User should enter <edges> pairs of directed edges\n"
           "            Kosaraju\n"
           "            Newedge <from>,<to>\n"
           "            Removeedge <from>,<to>\n"
           "        4: run as \"chat\" server using poll multiplexer API. connected clients can enter commands as in 3\n"
           "        6: as 4 using reactor pattern\n"
           "        7: run as \"chat\" server using a thread per client connection implementation\n"
           "        9: as 7 using proactor pattern\n"
           "       10: added monitor thread on top of 9 to monitor existing scc crossing 50%% threshold of vertices\n");

    exit(-1);
}

void getAndExecuteCommand()
{
    char input[1024];
    printCommands();
    while (1)
    {
        ssize_t bytesRead;
        bytesRead = read(STDIN_FILENO, input, sizeof(input) - 1);
        if (bytesRead < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        input[bytesRead] = '\0';
        executeCommand(input);
    }
}

int main(int argc, char *argv[])
{
    void *r;
    int vertices, edges, stage;

    if (argc != 2)
    {
        usage();
    }
    stage = atoi(argv[1]);
    switch (stage)
    {
    case 1:
        printf("Enter the number of vertices and number of edges:");
        scanf("%d,%d", &vertices, &edges);
        globalGraph = getNewGraph(vertices, edges);
        kosaraju(globalGraph);
        freeGraph(globalGraph);
        break;
    case 3:
        printf("execution mode: interaction\n");
        getAndExecuteCommand();
        break;
    case 4:
        printf("execution mode: multi user\n");
        chat(PORT);
        break;
    case 6:
        printf("execution mode: multi user reactor\n");
        r = createtReactor();
        createAndAddListnerToReactor(PORT, r);
        startReactor(r);
        printf("reactor %ld\n", (long int)r);
        break;
    case 7:
        printf("execution mode: multi user thread per client\n");
        acceptAndCreateThreadPerClients(PORT);
        break;
    case 10:
        printf("execution mode: monitor large scc changes\n");
        startMonitorLargeSCCChanges();
        __attribute__((fallthrough));
    case 9:
        printf("execution mode: proactor\n");
        void *proactor = createAndAddListnerToProactor(PORT);
        getchar();
        shutdownProactor(proactor);
        if (stage == 10)
        {
            signalMonitorLargeSCCChangesToTerminate();
        }
        sleep(10);
        break;
    default:
        usage();
    }

    return 0;
}
