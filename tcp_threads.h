#ifndef __TCP_THREADS_H__
#define __TCP_THREADS_H__

extern pthread_mutex_t _mutex;
void acceptAndCreateThreadPerClients(const char *port);
#endif // __TCP_THREADS_H__