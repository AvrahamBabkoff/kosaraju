#Kosaraju

Kosaraju

This project implements all stages of Exercise 3.

Stage 1
Files: kosaraju.c, main.c, Makefile
The directed graph implementation uses adjacency lists.

Stage 3
Files: kosaraju.c, main.c
Added removeEdge method and a method to receive, parse, and execute commands.
All interactions use stdin and stdout.

Stage 4
Files: main.c, pollserver.c, tcp_dup.c, Makefile
Based on Beej's chat implementation, together with duplicating stdin and stdout with the client sockets to meet the requirements.

Stage 5
Files: poll_reactor.c, Makefile
Created a static library async_engine.a implementing the reactor pattern.

Although the reactor can execute on the main thread, we chose to have the reactor poll loop run on a dedicated thread. This allows us to demonstrate the stopReactor method in the next stage (stage 6).

The stopReactor method uses a pipe to communicate the stop request to the main thread.
Note: To achieve this, we slightly modified the interface as follows:

Added method void *createReactor(); to create the internal reactor data structure.
Changed int startReactor(void *reactor_instance); to receive the reactor instance as a parameter.
Stage 6
Files: reactor_impl.c, Makefile
Using the library, this stage implements two reactor functions for:

Newly connected clients. The method registers the "data received" reactor functions for the newly connected client.
"Data received" reactor function. This method interacts with the command functionality implemented in Stage 3.
The user will be prompted to "enter any key to terminate." When a key is entered, the stopReactor method is invoked.
Stage 7
Files: tcp_threads.c, Makefile
This stage implements an infinite loop blocking on accept. Upon accept returning with a new client socket, a new "receive from client" thread is created with the new client socket sent as the thread argument. The "receive from client" thread implements an infinite loop waiting on recv, and when data is received, it interacts with the command functionality implemented in Stage 3.

Stage 8
Files: proactor.c, Makefile
Added an implementation of the proactor pattern to the async_engine.a library.
As with the reactor, we chose to have the proactor run on a dedicated thread. To implement the stopProactor, we made the following design decisions:

Use a pipe to signal the main thread.
Use the poll command on the listener socket and on the read end of the pipe.
This way, we can receive clients as in Stage 7 (using poll on the listener instead of accept), and receive a termination signal (using the pipe). The pipe is further used to allow disconnecting clients to communicate the disconnect event to the main thread for resource cleanup.
Note: We slightly modified the interface of the proactor as follows:

void *startProactor(int sockfd, proactorFunc threadFunc);
int stopProactor(void *proactor);
This allows us to use an abstract pointer to the proactor, similar to the reactor, instead of using pthread_t.
Stage 9
Files: main.c, tcp_threads.c
Implemented Stage 7 using the proactor functionality added in Stage 8. The user will be prompted to "enter any key to terminate." When a key is entered, the stopProactor method is invoked.

Stage 10
Files: main.c, kosaraju.c
Starts a thread that monitors the crossing of the 50% threshold of vertices belonging to a single SCC. The thread waits on a pthread_cond_t. The kosaraju method signals the pthread_cond_t when the threshold crossing occurs.

General Notes
We use a single Makefile for both the executable and the static library.
This single executable implements stages 1, 3, 4, 5, 6, 7, 8, 9, and 10 of the exercise.
To run the program from the project folder:
Compile: make all
Run: bin/kosaraju <stage> where <stage> can be one of: 1, 3, 4, 5, 6, 7, 8, 9, 10
We define _GNU_SOURCE for the project to use the pthread_timedjoin_np method. (https://linux.die.net/man/3/pthread_timedjoin_np)