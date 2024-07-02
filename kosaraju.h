#ifndef __KOSARAJU_H__
#define __KOSARAJU_H__

void createGraphAndExecuteKosaraju(int vertices, int edges);
void executeCommand(char *command);
void printCommands();
void startMonitorLargeSCCChanges();
void signalMonitorLargeSCCChangesToTerminate();

#endif // __KOSARAJU_H__