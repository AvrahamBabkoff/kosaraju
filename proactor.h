#ifndef __PROACTOR_H__
#define __PROACTOR_H__


typedef void (*proactorFunc) (int sockfd);

void * startProactor (int sockfd, proactorFunc threadFunc);

int stopProactor(pthread_t tid);

#endif
