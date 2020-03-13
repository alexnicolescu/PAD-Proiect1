#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include "netio.h"
#define SERVERADDRESS "127.0.0.1" //Valoarea trebuie inlocuita cu adresa calculatorului pe care ruleaza serverul
#define SERVERPORT 5678

int sockfd,nread,errorCode=-1;
char buf[1024],buf2[1024];
struct sockaddr_in local_addr,remote_addr;
void connectToServer()
{
	if((sockfd=socket(PF_INET,SOCK_STREAM,0))==-1)
	{
		perror("Unable to create a socket");
		exit(errorCode--);

	}
	set_addr(&local_addr,NULL,INADDR_ANY,0);
	if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr))==-1)
	{
		perror("Unable to bind a name to the socket");
		exit(errorCode--);	
	}
	set_addr(&remote_addr,SERVERADDRESS,0,SERVERPORT);
	if(connect(sockfd,(struct sockaddr*)&remote_addr,sizeof(remote_addr))==-1)
	{
		perror("Unable to connect to the socket");
		exit(errorCode--);
	}
}


int main(int argc,char*argv[]){
	
	connectToServer();
	printf("Mesesage:\n");
	fgets(buf,1024,stdin);
	buf[strlen(buf)-1]='\0';
	if(send(sockfd,buf,strlen(buf),0)==-1)
	{
		perror("Unable to send the message");
		exit(errorCode--);
	}
	while(1){
		nread=read(sockfd,(void*)buf2,1024);
		if(nread<=0)
		{
			perror("Unable to read/No more information to read");
			exit(errorCode--);
		}
		buf2[nread]='\0';
		if(nread>0)
			printf("%s\n",buf2);}
		if(close(sockfd)==-1)
		{
			perror("Unable to close the socket");
			exit(errorCode--);
		}
		exit(0);
	}
