#ifndef NETIO
#define NETIO
#include <stdint.h>
int set_addr(struct sockaddr_in *addr,char *name,uint32_t inaddr,short sin_port);
int stream_read(int sockfd,char*buf,int len);
int stream_write(int sockfd,char*buf,int len);
void check_error(int action, char *message);
#endif