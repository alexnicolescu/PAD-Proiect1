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

int main(int argc,char*argv[]){
	int sockfd;
	char buf[1024],buf2[1024];
	struct sockaddr_in local_addr,remote_addr;
	int nread;
	printf("Mesajul de transmis:\n");
	fgets(buf,1024,stdin);
	buf[strlen(buf)-1]='\0';
	sockfd=socket(PF_INET,SOCK_STREAM,0);
	set_addr(&local_addr,NULL,INADDR_ANY,0);
	bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr));
	set_addr(&remote_addr,SERVERADDRESS,0,SERVERPORT);
	connect(sockfd,(struct sockaddr*)&remote_addr,sizeof(remote_addr));
	send(sockfd,buf,strlen(buf),0);
	nread=read(sockfd,(void*)buf2,1024);
	buf2[nread]='\0';
	printf("%s\n",buf2);
	close(sockfd);
	exit(0);
}
