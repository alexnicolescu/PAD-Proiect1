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
void getName(char *info,char *name)
{
  int i=0;
  while(1)
  {
    if(info[i]==' ')
    {
      break;
    }
    name[i]=info[i];
    i++;
  }
  name[i]='\0';
}
void getPass(char *info,char *pass)
{
  int n=strlen(info);
  int ok=0,i=0,j=0;
  while(i!=n)
  {
    if(info[i]==' ')
    {
      ok=1;
    }
    if(ok==1)
    {
      pass[j]=info[i];
      j++;
    }
    i++;
  }
  pass[j]='\0';
}
int searchName(FILE *f,char *name)
{
  char info[256];
  char cname[100];
  while(fgets(info,256,f))
  {
    info[strlen(info)-1]='\0';
    getName(info,cname);
    if(strcmp(name,cname)==0)
      return 1;
  }
  fseek(f,0,SEEK_SET);
  return 0;

}
void validName(char *name,FILE *f)
{
  int option;
  while(searchName(f,name))
  {
    option=-1;
    if(send(connfd,(void*)(&option),sizeof(int),0)==-1)
    {
      perror("Unable to send the name already exists code");
      exit(errorCode--);
    }
    strcpy(name,"");
    nread=read(connfd,(void*)name,100);
    if(nread<0)
    {
      perror("Unable to read the name from socket");
      exit(errorCode--);
    }
    name[nread]='\0';
  }
  option=1;
  if(send(connfd,(void*)(&option),sizeof(int),0)==-1)
  {
    perror("Unable to send the name is valid code");
    exit(errorCode--);
  }
}
int existingUser(char *user,FILE *f)
{
  char info[256];
  while(fgets(info,256,f))
  {
    info[strlen(info)-1]='\0';
    if(strcmp(user,info)==0)
    {
      return 1;
    }
  }
  fseek(f,0,SEEK_SET);
  return 0;
}
void validUser(char *info,FILE *f)
{
  int option;
  while(!existingUser(info,f))
  {
    option=-1;
    if(send(connfd,(void*)(&option),sizeof(int),0)==-1)
    {
      perror("Unable to send the user doesn't exist code");
      exit(errorCode--);
    }
    strcpy(info,"");
    nread=read(connfd,(void*)info,256);
    if(nread<0)
    {
      perror("Unable to read the credentials");
      exit(errorCode--);
    }
    info[nread]='\0';
  }
  option=1;
  if(send(connfd,(void*)(&option),sizeof(int),0)==-1)
  {
    perror("Unable to send the user is valid code");
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
    int option;
    FILE *f;
    char name[100],pass[100],info[256];
    nread=read(connfd,(void*)(&option),sizeof(int));
    if(nread<0)
    {
      perror("Unable to read information from the socket");
      exit(errorCode--);
    }
    f=fopen("users.txt","r");
    if(f==NULL)
    {
      perror("Unable to open the users file");
      exit(errorCode--);
    }
    if(option==1)
    {
      nread=read(connfd,(void*)name,100);
      if(nread<0)
      {
        perror("Unable to read the name from socket");
        exit(errorCode--);
      }
      name[nread]='\0';
      validName(name,f);
      nread=read(connfd,(void*)pass,100);
      if(nread<0)
      {
        perror("Unable to read the name from socket");
        exit(errorCode--);
      }
      pass[nread]='\0';
      if(fclose(f)!=0)
      {
        perror("Unable to close the users file");
        exit(errorCode--);
      }
      f=fopen("users.txt","a");
      if(!f)
      {
        perror("Unable to open the user.txt file");
        exit(errorCode--);
      }
      int size;

      size=strlen(name)+strlen(pass) + 2;
      snprintf(info,size,"%s %s",name,pass);
      fwrite(info,strlen(info),1,f);
      fputc('\n',f);
      // fputs(info,f);
      if(fclose(f)!=0)
      {
        perror("Unable to close the users file");
        exit(errorCode--);
      }

    }
    else
    {
      if(option==2)
      {
        nread=read(connfd,(void*)info,256);
        if(nread<0)
        {
          perror("Unable to read the credentials");
          exit(errorCode--);
        }
        info[nread]='\0';
        validUser(info,f);
        if(fclose(f)!=0)
        {
          perror("Unable to close the users file");
          exit(errorCode--);
        }
        getName(info,name);
        getPass(info,pass);
      }
    }
    nread=read(connfd,(void*)buf,1024);
    if(nread<=0)
    {
      perror("Unable to receive information from client");
      exit(errorCode--);
    }
    buf[nread]='\0';
    printf("%s\n",buf);
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
