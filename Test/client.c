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
#define SERVERADDRESS "127.0.0.1" //Valoarea trebuie inlocuita cu adresa calculatorului pe care ruleaza serverul
#define SERVERPORT 5678

int sockfd,nread,errorCode=-1,n=0;
char buf[1024],buf2[1024],s1[1024],sir[1024];
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
char name[100],pass[100];

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


void login() {
	FILE *users_fd;
	char names[100][100],passwords[100][100];
	int index=0;
	users_fd=fopen("users.txt","a+");
	if(users_fd==NULL)
	{
		perror("File not found");
		exit(-1);

	}
	int option=0;
	printf("1: Cont nou \n2: Cont existent\n");
	scanf("%d",&option);
	fflush(stdin);
	if(option==1) {
		printf("Nume:\n");
		scanf("%s",name);
		printf("Pass:\n");
		scanf("%s",pass);
		fprintf(users_fd,"%s;%s;\n",name,pass);
	}
	else if (option==2) {
		int gasit=0;
		while(fscanf(users_fd,"%[^;];%[^;];\n",names[index],passwords[index])!=EOF) {
			index++;
		}
		while (!gasit) {
			printf("Introduceti un nume salvat: ");
			scanf("%s",name);
			for (int i=0;i<index;i++) {
				if(!strcmp(name,names[i])) {
					gasit=1;
					index=i;
				}
			}
		}
		while(strcmp(pass,passwords[index])) {
			printf("Introduceti parola: ");
			scanf("%s",pass);
		}
		fseek(users_fd, 0, SEEK_SET);
	}
	fclose(users_fd);
}


void readMessage()
{
    while(1)
    {
      nread=read(sockfd,(void*)buf2,1024);
      if(nread<=0)
      {
       perror("Unable to read/No more information to read");
       exit(errorCode--);
      }
      buf2[nread]='\0';
      if(nread>0)
      {
        for(int i=0;i<strlen(buf2);i++)
           { 
            if(buf2[i]=='\n')
              {
               sir[n]='\0';
               printf("%s\n",sir);
               n=0;
               strcpy(sir,"");
              }
             else
              {
               sir[n]=buf2[i];
               n++;
              }
           }
      }
    }
}

void writeMessage()
{
   while(1)
   { fflush(stdin);
     fflush(stdout);
     fgets(buf,1024,stdin);
     buf[strlen(buf)-1]='\0';
     strcpy(s1,name);
     strcpy(s1,strcat(s1,": "));
     strcpy(buf,strcat(s1,buf));
     strcat(buf,"\n");
     if(send(sockfd,buf,strlen(buf),0)==-1)
     {perror("Unable to send the message");
      exit(errorCode--); 
     }
   }
}

int main(int argc,char*argv[]){	
        connectToServer();
	login();
        getchar();
        pid=fork();
	if(pid<0)
	{
		perror("Unable to create a child process");
		exit(errorCode--);
	}
	if(pid==0){
           readMessage();
           exit(0);}
                   
        
	writeMessage();	
	
        if(close(sockfd)==-1)
        {
         perror("Unable to close the socket");
         exit(errorCode--);
        }
}
