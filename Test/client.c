#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include "netio.h"
#include <sys/wait.h>
#define SERVERADDRESS "127.0.0.1" //Valoarea trebuie inlocuita cu adresa calculatorului pe care ruleaza serverul
#define SERVERPORT 5678

int sockfd, nread, errorCode = -1, n = 0;
char buf[1024], buf2[1024], s1[1024], sir[1024];
struct sockaddr_in local_addr, remote_addr;
pid_t pid;
int logged_in = 0;

void connectToServer()
{
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Unable to create a socket");
		exit(errorCode--);
	}
	set_addr(&local_addr, NULL, INADDR_ANY, 0);
	if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)
	{
		perror("Unable to bind a name to the socket");
		exit(errorCode--);
	}
	set_addr(&remote_addr, SERVERADDRESS, 0, SERVERPORT);
	if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) == -1)
	{
		perror("Unable to connect to the socket");
		exit(errorCode--);
	}
}

void sigchldHandler(int signal)
{
	int status;
	wait(&status);
	if (!WIFEXITED(status))
	{
		perror("Child process ended with an error");
		exit(errorCode--);
	}
	if (close(sockfd) == -1)
	{
		perror("Unable to close the socket");
		exit(errorCode--);
	}
	exit(0);
}
char name[100], pass[100];

void handleSIGCHLD()
{
	struct sigaction actionForSIGCHLD;
	actionForSIGCHLD.sa_flags = 0;
	sigemptyset(&(actionForSIGCHLD.sa_mask));
	actionForSIGCHLD.sa_handler = sigchldHandler;
	if (sigaction(SIGCHLD, &actionForSIGCHLD, NULL) < 0)
	{
		perror("SIGCHLD error");
		exit(errorCode--);
	}
}

void login()
{
	char buf[256];
	int option = 0;
	printf("1: Cont nou \n2: Cont existent\n");
	scanf("%d", &option);
	fflush(stdin);
	while (logged_in == 0)
	{
		printf("Nume:\n");
		scanf("%s", name);
		printf("Pass:\n");
		scanf("%s", pass);
		if(option==1)
			getchar();
		char message[256];
		snprintf(message, 255, "%d;%s;%s;", option, name, pass);
		check_error(send(sockfd, (void *)message, strlen(message), 0), "Can't send credentials");
		check_error(recv(sockfd, buf, sizeof(int), 0), "Can't receive credentials");
		logged_in=atoi(buf);
	}
}

void readMessage()
{
	while (1)
	{
		nread = read(sockfd, (void *)buf2, 1024);
		if (nread <= 0)
		{
			perror("Unable to read/No more information to read");
			exit(errorCode--);
		}
		buf2[nread] = '\0';
		if (nread > 0)
		{
			for (int i = 0; i < strlen(buf2); i++)
			{
				if (buf2[i] == '\n')
				{
					sir[n] = '\0';
					printf("%s\n", sir);
					n = 0;
					strcpy(sir, "");
				}
				else
				{
					sir[n] = buf2[i];
					n++;
				}
			}
		}
	}
}

void writeMessage()
{
	while (1)
	{
		fflush(stdin);
		fflush(stdout);
		fgets(buf, 1024, stdin);
		buf[strlen(buf) - 1] = '\0';
		if(strcmp(buf,"exit")==0)
		{	
			if(kill(pid,SIGUSR1))
			{
				perror("Unable to send SIGURS1 signal to the child process");
				exit(errorCode--);
			}
			break;
		}
		strcpy(s1, name);
		strcpy(s1, strcat(s1, ": "));
		strcpy(buf, strcat(s1, buf));
		strcat(buf, "\n");
		if (send(sockfd, buf, strlen(buf), 0) == -1)
		{
			perror("Unable to send the message");
			exit(errorCode--);
		}
	}
}
void handlerSIGUSR1(int signal){
	if(close(sockfd)==-1){
		perror("Unable to close the socket in the child process");
		exit(errorCode--);
	}
	exit(0);
}
void listenForSIGUSR1(){
	struct sigaction sig;
	sig.sa_flags=0;
	sigemptyset(&(sig.sa_mask));
	sig.sa_handler=handlerSIGUSR1;
	if(sigaction(SIGUSR1,&sig,NULL)<0)
	{
		perror("SIGUSR1 error");
		exit(errorCode--);
	}
}

int main(int argc, char *argv[])
{
	connectToServer();
	login();
	printf("Bine ai venit!\n");
	// getchar();
	pid = fork();
	if (pid < 0)
	{
		perror("Unable to create a child process");
		exit(errorCode--);
	}
	if (pid == 0)
	{
		listenForSIGUSR1();
		readMessage();
	}
	handleSIGCHLD();
	writeMessage();
}
