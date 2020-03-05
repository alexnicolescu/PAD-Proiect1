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

#include "netio.h"

#define SERVER_PORT 5678

int main(void){
	int sockfd,connfd;
	char buf[1024];
        int fd,n,l,status;
	struct sockaddr_in local_addr,rmt_addr;
	int nread,p=0;
        char s[10];
	socklen_t rlen;
	sockfd=socket(PF_INET,SOCK_STREAM,0);
	set_addr(&local_addr,NULL,INADDR_ANY,SERVER_PORT);
	bind(sockfd,(struct sockaddr *)&local_addr,sizeof(local_addr));
	listen(sockfd,5);
	rlen=sizeof(rmt_addr);
        
        fd=open("a.txt",O_RDWR |O_APPEND |O_TRUNC);
         if(fd==-1)
          {printf("eroare deschidere fisier");
          }

        for(;;)
        {
         p++;
         connfd=accept(sockfd,(struct sockaddr *)&rmt_addr,&rlen);
         snprintf(s,sizeof(int),"%d",connfd);
         write(fd,s,strlen(s));
        
         if(fork()==0)
          {close(sockfd);
	   nread=read(connfd,(void*)buf,1024);
	   buf[nread]='\0';
	   printf("%s\n",buf);
	
	  if(nread<0)
	  {
	   printf("Eroare la citirea de la retea\n");
	  }
        
         lseek(fd,0,SEEK_SET);
         while(l=(read(fd,s,sizeof(char)))>0)
          {s[l]='\0';
           n=atoi(s);
	   write(n,(void*)buf,strlen(buf));
          }
	  exit(0);
        }
        }
     close(fd);
     close(connfd);
     for(int i=0;i<=p;i++)
      wait(&status);
}
