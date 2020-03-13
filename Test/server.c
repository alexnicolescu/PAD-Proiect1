#include <stdio.h>
#include<sys/types.h>
#include <sys/wait.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "netio.h"

#define SERVER_PORT 5678
int errorCode=-1;
int sockfd,connfd;
char buf[1024];
int fd,n,l,status;
struct sockaddr_in local_addr,rmt_addr;
int nread;
char s[10];
socklen_t rlen;
pid_t pid;

void prepareToReceiveRequests()
{
  if((sockfd=socket(PF_INET,SOCK_STREAM,0))==-1){
    perror("Unable to create a socket");
    exit(errorCode--);
  }
  set_addr(&local_addr,NULL,INADDR_ANY,SERVER_PORT);
  if(bind(sockfd,(struct sockaddr *)&local_addr,sizeof(local_addr))==-1)
  {
    perror("Unable to bind a name to the socket");
    exit(errorCode--);  
  }
  if(listen(sockfd,5)==-1)
  {
    perror("Unable to listen for connections on this socket");
    exit(errorCode--);
  }
  rlen=sizeof(rmt_addr);
}

void openFile()
{
  fd=open("socketDescriptors.txt",O_RDWR |O_APPEND |O_TRUNC);
  if(fd==-1)
  {
    perror("File not found");
    exit(errorCode--);

  }
}

int main(void)
{
  prepareToReceiveRequests();
  openFile();
  for(;;)
  {
   if((connfd=accept(sockfd,(struct sockaddr *)&rmt_addr,&rlen))==-1)
   {
    perror("Unable to accept a conection on this socket");
    exit(errorCode--);
  }
  snprintf(s,sizeof(int),"%d",connfd);
  if(write(fd,s,strlen(s))==-1)
  {
    perror("Unable to write the descriptor in the file");
    exit(errorCode--);
  }
  pid=fork();
  if(pid==-1)
  {
    perror("Unable to create a child process");
    exit(errorCode--);
  }
  if(pid==0)
  {
    if(close(sockfd)==-1)
    {
      perror("Unable to close the socket");
      exit(errorCode--);
    }
    nread=read(connfd,(void*)buf,1024);
    if(nread<=0)
    {
      perror("Unable to receive information from client");
      exit(errorCode--);
    }
    buf[nread]='\0';
    printf("Message from a client:%s\n",buf);
    lseek(fd,0,SEEK_SET);
    while((l=read(fd,(void*)s,sizeof(char)))>0)
    {
      s[l]='\0';
      n=atoi(s);
      if(write(n,(void*)buf,strlen(buf))==-1)
      {
        perror("Unable to send the message to other clients");
        exit(errorCode--);
      }
    }
    exit(0);
  }
}
if(close(fd)==-1)
{
  perror("Unable to close the file");
  exit(errorCode--);
}
if(close(connfd)==-1)
{
  perror("Unable to close the socket");
  exit(errorCode--);
}
}
