#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "kosaraju.h"
#include "pollserver.h"

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
           "        3: run as \"chat\" server. connected clients can enter commands as in 2\n"
           );

    exit(-1);
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
        // printf("found node to remove\n");
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
    // printf("adding edge %d, %d\n", src, dest);
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

void dfsPrint(Graph *graph, int vertex)
{
    graph->visited[vertex] = true;
    printf("%d ", vertex + 1);

    Node *temp = graph->adjLists[vertex];
    while (temp)
    {
        int adjVertex = temp->vertex;
        if (!graph->visited[adjVertex])
        {
            dfsPrint(graph, adjVertex);
        }
        temp = temp->next;
    }
}

void kosaraju(Graph *graph)
{
    int vertices = graph->numVertices;
    int stack[vertices];
    int stackIndex = 0;
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
            dfsPrint(transpose, v);
            printf("\n");
        }
    }
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
            int src, dest;
            scanf("%d %d", &src, &dest);
            addEdge(globalGraph, src, dest);
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
           "            Removeedge <from>,<to>\n");
}

void executeCommand(char *input)
{
    char *param1, *param2, *saveptr;

    char *token = strtok_r(input, " \n", &saveptr);

    if (token != NULL)
    {
        // printf("the token: %s\n", token);
        if (strcmp(token, "Newgraph") == 0)
        {
            getParameters(&param1, &param2, &saveptr);
            globalGraph = getNewGraph(atoi(param1), atoi(param2));
            // printf("parameters: %s, %s\n", param1, param2);
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
                addEdge(globalGraph, atoi(param1), atoi(param2));
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
                removeEdge(globalGraph, atoi(param1), atoi(param2));
            }
            else
            {
                printf("Graph does not exist, please create a graph\n");
            }
        }
        else
        {
            printf("unrecognized command aaa%saaa\n", token);
            usage();
        }
    }
}

void getAndExecuteCommand()
{
    // char *param1, *param2;
    char input[1024];
    printCommands();
    while (1)
    {
        printf("enter command:\n");
        if (fgets(input, sizeof(input) - 1, stdin) == NULL)
        {
            printf("error reading input \n");
            break;
        }
        executeCommand(input);

#if 0        
        // printf("user input: %s\n", input);
        char *token = strtok(input, " \n");

        if (token != NULL)
        {
            // printf("the token: %s\n", token);
            if (strcmp(token, "Newgraph") == 0)
            {
                getParameters(&param1, &param2);
                globalGraph = getNewGraph(atoi(param1), atoi(param2));
                // printf("parameters: %s, %s\n", param1, param2);
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
                    getParameters(&param1, &param2);
                    addEdge(globalGraph, atoi(param1), atoi(param2));
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
                    getParameters(&param1, &param2);
                    removeEdge(globalGraph, atoi(param1), atoi(param2));
                }
                else
                {
                    printf("Graph does not exist, please create a graph\n");
                }
            }
            else
            {
                printf("unrecognized command aaa%saaa\n", token);
                usage();
            }
        }
#endif
    }
}

int main(int argc, char *argv[])
{
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
    default:
        usage();
    }

    printf("Strongly Connected Components:\n");

    return 0;
}
