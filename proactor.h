#ifndef __PROACTOR_H__
#define __PROACTOR_H__


typedef void * (*proactorFunc) (int sockfd);

void * startProactor (int sockfd, proactorFunc threadFunc);

int stopProactor(void *proactor);

#endif
