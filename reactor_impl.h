#ifndef __TCP_REACTOR_H__
#define __TCP_REACTOR_H__

#include "poll_reactor.h"
void createAndAddListnerToReactor(const char *port, void *reactor);
#endif // __TCP_REACTOR_H__