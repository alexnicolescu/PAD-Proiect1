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
#include <sys/wait.h>
#include <ctype.h>
#define SERVERADDRESS "127.0.0.1" //Valoarea trebuie inlocuita cu adresa calculatorului pe care ruleaza serverul
#define SERVERPORT 5678

int sockfd,nread,errorCode=-1;
char buf[1024],buf2[1024];
struct sockaddr_in local_addr,remote_addr;
pid_t pid;

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

void sigchldHandler(int signal)
{
	int status;
	wait(&status);
	if(!WIFEXITED(status))
	{
		perror("Child process ended with an error");
		exit(errorCode--);
	}
}
char name[100],pass[100],info[256];

void handleSIGCHLD()
{
	struct sigaction actionForSIGCHLD;
	actionForSIGCHLD.sa_flags=0;
	sigemptyset(&(actionForSIGCHLD.sa_mask));
	actionForSIGCHLD.sa_handler=sigchldHandler;
	if(sigaction(SIGCHLD,&actionForSIGCHLD,NULL)<0)
	{
		perror("SIGCHLD error");
		exit(errorCode--);
	}
}
void nameAlreadyExists(int option)
{
	while(option==-1)
	{
		strcpy(name,"");
		printf("Name already exists,enter a new one:\n");
		scanf("%s",name);
		fflush(stdin);
		if(send(sockfd,(void*)name,strlen(name),0)==-1)
		{
			perror("Unable to send the name via socket");
			exit(errorCode--);
		}
		nread=read(sockfd,(void*)(&option),sizeof(int));
		if(nread<0)
		{
			perror("Unable to read name already exists code");
			exit(errorCode--);
		}
	}
}
void existingUser(int option)
{
	int size;
	while(option==-1)
	{
		strcpy(info,"");
		strcpy(name,"");
		strcpy(pass,"");
		printf("User doesn't exist\n");
		printf("Name:\n");
		scanf("%s",name);
		printf("Pass:\n");
		scanf("%s",pass);
		size=strlen(name)+strlen(pass)+2;
		snprintf(info,size,"%s %s",name,pass);
		if(send(sockfd,(void*)info,strlen(info),0)==-1)
		{
			perror("Unable to send the credentials via socket");
			exit(errorCode--);
		}
		nread=read(sockfd,(void*)(&option),sizeof(int));
		if(nread<0)
		{
			perror("Unable to read name already exists code");
			exit(errorCode--);
		}
	}

}

void login() {
	printf("Select an option\n1.Create a new account\n2.Login using an existing account\n");
	int option;
	scanf("%d",&option);
	fflush(stdin);
	if(send(sockfd,(void*)(&option),sizeof(option),0)==-1)
	{
		perror("Unable to use the socket");
		exit(errorCode--);
	}
	if(option==1)
	{
		printf("Name:\n");
		scanf("%s",name);
		fflush(stdin);
		if(send(sockfd,(void*)name,strlen(name),0)==-1)
		{
			perror("Unable to send the name via socket");
			exit(errorCode--);
		}
		nread=read(sockfd,(void*)(&option),sizeof(int));
		if(nread<0)
		{
			perror("Unable to read the name already exists code");
			exit(errorCode--);
		}
		nameAlreadyExists(option);
		printf("Pass:\n");
		scanf("%s",pass);
		if(send(sockfd,(void*)pass,strlen(pass),0)==-1)
		{
			perror("Unable to send the password via socket");
			exit(errorCode--);
		}
		
	}
	else
	{
		if(option==2)
		{
			printf("Name:\n");
			scanf("%s",name);
			printf("Pass:\n");
			scanf("%s",pass);
			fflush(stdin);
			int size=strlen(name)+strlen(pass)+2;
			snprintf(info,size,"%s %s",name,pass);
			if(send(sockfd,(void*)info,strlen(info),0)==-1)
			{
				perror("Unable to send the credentials");
				exit(errorCode--);
			}
			nread=read(sockfd,(void*)(&option),sizeof(int));
			if(nread<0)
			{
				perror("Unable to read the name already exists code");
				exit(errorCode--);
			}
			existingUser(option);
		}
	}
	fflush(stdin);
	fflush(stdout);
	
	
}

int main(int argc,char*argv[]){
	connectToServer();
	login();
	pid=fork();
	if(pid<0)
	{
		perror("Unable to create a child process");
		exit(errorCode--);
	}
	if(pid==0)
	{
		fflush(stdin);
		fflush(stdout);
		getchar();
		printf("Introduceti mesajul: ");
		fgets(buf,1024,stdin);
		buf[strlen(buf)-1]='\0';
		strcpy(name,strcat(name,": "));
		strcpy(buf,strcat(name,buf));
		if(send(sockfd,buf,strlen(buf),0)==-1)
		{
			perror("Unable to send the message");
			exit(errorCode--);
		}
		exit(0);
	}
	handleSIGCHLD();
	while(1){
		sleep(2);
		nread=read(sockfd,(void*)buf2,1024);
		if(nread<0)
		{
			perror("Unable to read/No more information to read");
			exit(errorCode--);
		}
		buf2[nread]='\0';
		if(nread>0){

			printf("%s\n",buf2);}
		}
		if(close(sockfd)==-1)
		{
			perror("Unable to close the socket");
			exit(errorCode--);
		}
		exit(0);
	}
