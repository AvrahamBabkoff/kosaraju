# kosaraju
kosaraju

implements all stages of excersize 3

stage 1: kosaraju.c, main,c, makefile
The directed graph implementation uses an adjacency lists

stage 3: kosaraju.c, main.c
added removeEdge method and method to receive parse and execute the commands
all interaction uses stdin and stdout

stage 4: main.c, pollserver.c, tcp_dup.c, makefile
based on  beej chat implementation together with dupping stdin and stdout with the client sockets achieves the requirements

stage 5: poll_reactor.c, makefile
create a static lib async_engine.a implementing the reactor pattern

Although the reactor can execute on the main thread, we chose to have the reactor poll loop run on a dedicated thread, this, so that in the next stage we will be able to demonstrate the stopReactor mothod.

The stopReactor method uses a pipe to communicate the stop request to the main thread

stage 6: reactor_impl.c, makefile
using the library, this stage implements the main reactor functions: for newly connected clients and for data received from clients, the latter interacts with commands functionality implemented in stage 3

stage 7: tcp_threads.c, makefile
this stage implements an infinite loop blocking on accept. Upon accept returning with a new client socket, a new thread is created with the new client socket sent as the thread argument. the thread implements in infinite loop waiting on recv, and when data is received, it interacts with commands functionality implemented in stage 3

stage 8: proactor.c, makefile
add an implementation of the proactor pattern to async_engine.a
as with the reactor,  we chose to have the proactor run on its own thread. In addition, in order to be able to implement the 
stopProactor, we made the following implementation design decisions:
1) use a pipe to signal the main thread
2) use the poll command on the listner and the read end of the pipe
this way we have the benifit of being able to receive clients as in stage 7, and to be able to receive a termination signal
this pipe is further used to allow disconnecting clients to communicate the disconnect event to the main thread to allow for resource cleanup