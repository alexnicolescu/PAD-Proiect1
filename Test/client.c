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
#include"netio.h"
#define SERVERADDRESS "127.0.0.1" //Valoarea trebuie inlocuita cu adresa calculatorului pe care ruleaza serverul
#define SERVERPORT 5678

int main(int argc,char*argv[]){
	int fd,sockfd;
	char buf[1024];
	struct sockaddr_in local_addr,remote_addr;
	int nread;
	printf("Mesajul de transmis:");
	fgets(buf,1024,stdin);
	sockfd=socket(PF_INET,SOCK_STREAM,0);
	set_addr(&local_addr,NULL,INADDR_ANY,0);
	bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr));
	set_addr(&remote_addr,SERVERADDRESS,0,SERVERPORT);
	connect(sockfd,(struct sockaddr*)&remote_addr,sizeof(remote_addr));
	stream_write(sockfd,(void*)buf,strlen(buf));
	close(sockfd);
	exit(0);
}
