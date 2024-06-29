#ifndef __TCP_DUP_H__
#define __TCP_DUP_H__
int tcp_dup_std(int fd, int std);
void tcp_undup_std(int saved_stdout, int std);
#endif // __TCP_DUP_H__