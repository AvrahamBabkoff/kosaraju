#ifndef __KOSARAJU_H__
#define __KOSARAJU_H__
#include <stdbool.h>

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

extern Graph *globalGraph;

void executeCommand(char *command);
void printCommands();
Graph *getNewGraph(int vertices, int edges);
void freeGraph(Graph *graph);
void startMonitorLargeSCCChanges();
void kosaraju(Graph *graph);

#endif // __KOSARAJU_H__