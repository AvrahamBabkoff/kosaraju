#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "kosaraju.h"
#include "pollserver.h"
#include "poll_reactor.h"
#include "tcp_reactor.h"
#include "tcp_threads.h"

#define PORT "9034"

pthread_cond_t _cond = PTHREAD_COND_INITIALIZER;

bool sccThreasholdChanged = false;

typedef struct Node
{
    int vertex;
    struct Node *next;
} Node;

typedef struct Graph
{
    int numVertices;
    Node **adjLists;
    bool *visited;
    bool maxInSccMoreThan50Percent;
} Graph;

Graph *globalGraph = NULL;

void usage(void)
{
    printf("Usage: kosaraju <stage>\n"
           "    valid values of <stage>:\n"
           "        1: user enters graph vertices and edges, followed by a list of the directed edges\n"
           "        2: user can enter following commands:\n"
           "            Newgraph <verttices>,<edges>\n"
           "                User should enter <edges> pairs of directed edges\n"
           "            Kosaraju\n"
           "            Newedge <from>,<to>\n"
           "            Removeedge <from>,<to>\n"
           "        3: run as \"chat\" server. connected clients can enter commands as in 2\n");

    exit(-1);
}

void *MonitorLargeSCCChanges(void *arg)
{
    (void)arg;

    while (1)
    {
        printf("going to lock the mutex\n");
        pthread_mutex_lock(&_mutex);
        printf("got lock on mutex\n");
        if (!sccThreasholdChanged)
        {
            printf("going to wait on condition\n");

            pthread_cond_wait(&_cond, &_mutex);
            printf("returned from wait on condition\n");
        }
        if (sccThreasholdChanged)
        {
            printf(globalGraph->maxInSccMoreThan50Percent ? "At least 50%% of the graph belongs to the same SCC\n" : "At least 50%% of the graph no longer belongs to the same SCC\n");
            sccThreasholdChanged = false;
        }
        printf("going to unlock the mitext\n");
        pthread_mutex_unlock(&_mutex);
    }

    return NULL;
}
Node *createNode(int vertex)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->vertex = vertex;
    newNode->next = NULL;
    return newNode;
}

Graph *createGraph(int vertices)
{
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    graph->numVertices = vertices;
    graph->maxInSccMoreThan50Percent = false;

    graph->adjLists = (Node **)malloc(vertices * sizeof(Node *));
    graph->visited = (bool *)malloc(vertices * sizeof(bool));

    for (int i = 0; i < vertices; i++)
    {
        graph->adjLists[i] = NULL;
        graph->visited[i] = false;
    }

    return graph;
}

void freeGraph(Graph *graph)
{
    for (int i = 0; i < graph->numVertices; i++)
    {
        Node *temp = graph->adjLists[i], *next = NULL;
        while (temp)
        {
            next = temp->next;
            free(temp);
            temp = next;
        }
    }

    free(graph->adjLists);
    free(graph->visited);
    free(graph);
}

void removeEdge(Graph *graph, int src, int dest)
{
    Node *temp = graph->adjLists[src - 1];
    Node *prev = NULL;
    if (temp && temp->vertex == dest - 1)
    {
        graph->adjLists[src - 1] = temp->next;
        free(temp);
    }
    else
    {
        while (temp)
        {
            prev = temp;
            temp = temp->next;
            if (temp && temp->vertex == dest - 1)
            {
                prev->next = temp->next;
                free(temp);
                break;
            }
        }
    }
}

void addEdge(Graph *graph, int src, int dest)
{
    Node *newNode = createNode(dest - 1);
    newNode->next = graph->adjLists[src - 1];
    graph->adjLists[src - 1] = newNode;
}

void dfs(Graph *graph, int vertex, int *stack, int *stackIndex)
{
    graph->visited[vertex] = true;

    Node *temp = graph->adjLists[vertex];

    while (temp)
    {
        int adjVertex = temp->vertex;
        if (!graph->visited[adjVertex])
        {
            dfs(graph, adjVertex, stack, stackIndex);
        }
        temp = temp->next;
    }

    stack[(*stackIndex)++] = vertex;
}

Graph *getTranspose(Graph *graph)
{
    Graph *transpose = createGraph(graph->numVertices);

    for (int v = 0; v < graph->numVertices; v++)
    {
        Node *temp = graph->adjLists[v];
        while (temp)
        {
            addEdge(transpose, temp->vertex + 1, v + 1);
            temp = temp->next;
        }
    }

    return transpose;
}

void dfsPrint(Graph *graph, int vertex, int *cnt)
{
    if (cnt)
    {
        ++(*cnt);
    }
    graph->visited[vertex] = true;
    printf("%d ", vertex + 1);

    Node *temp = graph->adjLists[vertex];
    while (temp)
    {
        int adjVertex = temp->vertex;
        if (!graph->visited[adjVertex])
        {
            dfsPrint(graph, adjVertex, cnt);
        }
        temp = temp->next;
    }
}

void kosaraju(Graph *graph)
{
    int vertices = graph->numVertices;
    int stack[vertices];
    int stackIndex = 0;
    int verticesInScc = 0;
    int maxVerticesInScc = 0;
    for (int i = 0; i < vertices; i++)
    {
        graph->visited[i] = false;
    }

    printf("Strongly Connected Components:\n");
    for (int i = 0; i < vertices; i++)
    {
        if (!graph->visited[i])
        {
            dfs(graph, i, stack, &stackIndex);
        }
    }

    Graph *transpose = getTranspose(graph);

    for (int i = 0; i < vertices; i++)
    {
        transpose->visited[i] = false;
    }

    while (stackIndex > 0)
    {
        int v = stack[--stackIndex];
        if (!transpose->visited[v])
        {
            verticesInScc = 0;
            dfsPrint(transpose, v, &verticesInScc);
            printf("\n");
            maxVerticesInScc = verticesInScc > maxVerticesInScc ? verticesInScc : maxVerticesInScc;
        }
    }
    // printf("largest scc has %d vertices\n", maxVerticesInScc);
    // pthread_mutex_lock(&_mutex);
    if (maxVerticesInScc > (double)graph->numVertices / 2.0 && !graph->maxInSccMoreThan50Percent)
    {
        graph->maxInSccMoreThan50Percent = true;
        sccThreasholdChanged = true;
        // printf("At least 50%% of the graph belongs to the same SCC\n");
    }
    else if (maxVerticesInScc <= (double)graph->numVertices / 2.0 && graph->maxInSccMoreThan50Percent)
    {
        graph->maxInSccMoreThan50Percent = false;
        sccThreasholdChanged = true;
        // printf("At least 50%% of the graph no longer belongs to the same SCC\n");
    }
    if (sccThreasholdChanged)
    {
        pthread_cond_signal(&_cond);
    }
    // pthread_mutex_unlock(&_mutex);
    freeGraph(transpose);
}

Graph *getNewGraph(int vertices, int edges)
{
    if (globalGraph != NULL)
    {
        freeGraph(globalGraph);
        globalGraph = NULL;
    }
    globalGraph = createGraph(vertices);
    if (edges > 0)
    {
        printf("enter the %d directed edges as pairs of vertices <from> <to>:\n", edges);
        for (int i = 0; i < edges; i++)
        {
            char input[1024];
            ssize_t bytesRead;
            bytesRead = read(STDIN_FILENO, input, sizeof(input) - 1);
            if (bytesRead < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            input[bytesRead] = '\0';

            char *src, *dest, *saveptr;
            src = strtok_r(input, ",\n", &saveptr);
            dest = strtok_r(NULL, ",\n", &saveptr);

            addEdge(globalGraph, atoi(src), atoi(dest));
        }
    }
    return globalGraph;
}

void getParameters(char **param1, char **param2, char **saveptr)
{
    char *input = strtok_r(NULL, " ", saveptr);

    *param1 = strtok_r(input, ",\n", saveptr);
    *param2 = strtok_r(NULL, ",", saveptr);
}

void printCommands()
{
    printf("enter one of the following commands:\n"
           "            Newgraph <verttices>,<edges>\n"
           "                User should enter <edges> pairs of directed edges\n"
           "            Kosaraju\n"
           "            Newedge <from>,<to>\n"
           "            Removeedge <from>,<to>\n\n"
           "enter command:\n");
}

void executeCommand(char *input)
{
    char *param1 = NULL, *param2 = NULL, *saveptr;

    char *token = strtok_r(input, " \n", &saveptr);

    if (token != NULL)
    {
        if (strcmp(token, "Newgraph") == 0)
        {
            getParameters(&param1, &param2, &saveptr);
            if (param1 == NULL || param2 == NULL)
            {
                printf("Must specify verttices and edges\n");
            }
            else
            {
                globalGraph = getNewGraph(atoi(param1), atoi(param2));
            }
        }
        else if (strcmp(token, "Kosaraju") == 0)
        {
            if (globalGraph)
            {
                kosaraju(globalGraph);
            }
            else
            {
                printf("Graph does not exist, please create a graph\n");
            }
        }
        else if (strcmp(token, "Newedge") == 0)
        {
            if (globalGraph)
            {
                getParameters(&param1, &param2, &saveptr);

                if (param1 == NULL || param2 == NULL)
                {
                    printf("Must specify both endpoints of the new edge\n");
                }
                else
                {

                    addEdge(globalGraph, atoi(param1), atoi(param2));
                }
            }
            else
            {
                printf("Graph does not exist, please create a graph\n");
            }
        }
        else if (strcmp(token, "Removeedge") == 0)
        {
            if (globalGraph)
            {
                getParameters(&param1, &param2, &saveptr);

                if (param1 == NULL || param2 == NULL)
                {
                    printf("Must specify both endpoints of the edge to remove\n");
                }
                else
                {

                    removeEdge(globalGraph, atoi(param1), atoi(param2));
                }
            }
            else
            {
                printf("Graph does not exist, please create a graph\n");
            }
        }
        else
        {
            printf("unrecognized command %s\n", token);
            usage();
        }
    }
    printf("enter command:\n");
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
    pthread_t thread;
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
        scanf("%d %d", &vertices, &edges);
        globalGraph = getNewGraph(vertices, edges);
        kosaraju(globalGraph);
        freeGraph(globalGraph);
        break;
    case 2:
        getAndExecuteCommand();
        break;
    case 3:
        chat();
        break;
    case 4:
        r = createtReactor();
        createAndAddListnerToReactor(PORT, r);
        startReactor(r);
        printf("reactor %ld\n", (long int)r);
        break;
    case 6:
        printf("going to create thread\n");
        pthread_create(&thread, NULL, MonitorLargeSCCChanges, NULL);
        __attribute__ ((fallthrough));
    case 5:
        printf("execution mode: create thread per connected client\n");
        acceptAndCreateThreadPerClients(PORT);
        break;
    default:
        usage();
    }

    printf("Strongly Connected Components:\n");

    return 0;
}
