#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "netio.h"

#define SERVER_PORT 5678
int errorCode = -1;
int sockfd, connfd;
char buf[1024];
int fd, n, l, status;
struct sockaddr_in local_addr;
int nread;
char s[10];
socklen_t rlen;
pid_t pid;
typedef struct{
  struct sockaddr_in addr;
  int connfd;
}cli_t;


void prepareToReceiveRequests()
{
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Unable to create a socket");
    exit(errorCode--);
  }
  set_addr(&local_addr, NULL, INADDR_ANY, SERVER_PORT);
  if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)
  {
    perror("Unable to bind a name to the socket");
    exit(errorCode--);
  }
  if (listen(sockfd, 5) == -1)
  {
    perror("Unable to listen for connections on this socket");
    exit(errorCode--);
  }

}

void openFile()
{
  fd = open("socketDescriptors.txt", O_RDWR | O_APPEND | O_TRUNC | O_CREAT,0777);
  if (fd == -1)
  {
    perror("File not found");
    exit(errorCode--);
  }
}

cli_t* initializeConn()
{  int connfd;
 struct sockaddr_in rmt_addr;
 rlen=sizeof(rmt_addr);
 if ((connfd = accept(sockfd, (struct sockaddr *)&rmt_addr, &rlen)) == -1)
 {
  perror("Unable to accept a conection on this socket");
  exit(errorCode--);
}

cli_t *client=(cli_t *)malloc(sizeof(cli_t));
if(!client)
  {perror("Could not allocate");
exit(errorCode--);
}
client->connfd=connfd;
client->addr=rmt_addr;

snprintf(s, sizeof(int)+1, "%d\n", connfd);
if (write(fd, s, strlen(s)) == -1)
{
  perror("Unable to write the descriptor in the file");
  exit(errorCode--);
}

return client;
}

int option = 0;
char credentials[256];
char name[100], pass[100];
void get_credentials(cli_t* c)
{
  check_error(recv(c->connfd, credentials, 255, 0), "receive credentials");
  sscanf(credentials, "%d;%[^;];%[^;];", &option, name, pass);
}
int verify_name(FILE* users_fd)
{
  char cname[100],cpass[100];
  fseek(users_fd,0,SEEK_SET);
  while(fscanf(users_fd,"%[^;];%[^;];\n", cname, cpass)!=EOF)
  {
    if(strcmp(name,cname)==0)
      return 1;
  }
  return 0;
}
void login(cli_t* c)
{
  char res[10];
  FILE *users_fd;
  char names[100][100], passwords[100][100];
  int index = 0;
  users_fd = fopen("users.txt", "a+");
  if (users_fd == NULL)
  {
    perror("File not found");
    exit(-1);
  }
  get_credentials(c);
  if (option == 1)
    { while(verify_name(users_fd))
      {
        option=0;
        check_error(send(c->connfd,&option,sizeof(int),0),"Can't send option");
        get_credentials(c);
      }
      option=1;
      check_error(send(c->connfd,&option,sizeof(int),0),"Can't send option");
      fprintf(users_fd, "%s;%s;\n", name, pass);
      strcpy(res, "1");
      check_error(send(c->connfd, res, strlen(res), 0), "Can't send response");
    }
    else if (option == 2)
    {
      int gasit = 0;
      while (fscanf(users_fd, "%[^;];%[^;];\n", names[index], passwords[index]) != EOF)
      {
        index++;
      }
      while (!gasit)
      {
        for (int i = 0; i < index; i++)
        {
          if (!strcmp(name, names[i]) && !strcmp(pass, passwords[i]))
          {
            gasit = 1;
            strcpy(res, "1");
            check_error(send(c->connfd, res, strlen(res), 0), "Can't send response");
            break;
          }
        }
        strcpy(res, "0");
        check_error(send(c->connfd, res, strlen(res), 0), "Can't send response");
        get_credentials(c);
      }
      fseek(users_fd, 0, SEEK_SET);
    }
    fclose(users_fd);
  }
  void deleteDescriptorFromFile(int des)
  {
    FILE *f,*o;
    char str[100];
    f=fopen("socketDescriptors.txt","r");
    if(!f)
    {
      perror("Unable to open the file");
      exit(errorCode--);
    }
    o=fopen("test.txt","w");
    if(!o)
    {
      perror("Unable to open the file");
      exit(errorCode--);
    }
    while(fgets(str,100,f))
    {
      str[strlen(str)-1]='\0';
      if(atoi(str)!=des)
      {
        fputs(str,o);
        fputc('\n',o);
      }
      strcpy(s,"");
    }
    if(fclose(f)!=0)
    {
      perror("Unable to close the file");
      exit(errorCode);
    }
    if(fclose(o)!=0)
    {
      perror("Unable to close the file");
      exit(errorCode);
    }
    unlink("socketDescriptors.txt");
    rename("test.txt","socketDescriptors.txt");

    fd = open("socketDescriptors.txt", O_RDWR | O_APPEND);
    if (fd == -1)
    {
      perror("File not found");
      exit(errorCode--);
    }
  }

  void *child(void *arg)
  {
    cli_t *c=(cli_t *)arg;
    login(c);
    while (1)
    {
      strcpy(buf,"");
      nread = read(c->connfd, (void *)buf, 1024);
      if (nread <= 0)
      {
        pthread_exit(NULL);
      }
      buf[nread] = '\0';
      if(strcmp(buf,"exit")==0)
      {
        deleteDescriptorFromFile(c->connfd);
        pthread_exit(NULL);
      }
      // printf("%s\n", buf);
      lseek(fd, 0, SEEK_SET);
      while ((l = read(fd, (void *)s, sizeof(char))) > 0)
      {
        s[l] = '\0';
        n = atoi(s);
        if (write(n, (void *)buf, strlen(buf)) == -1)
        {
          perror("Unable to send the message to other clients");
          exit(errorCode--);
        }
      }
    }
    pthread_exit(NULL);
  }

  int main(void)
  {
    prepareToReceiveRequests();
    openFile();
    for (;;)
      { pthread_t tid;
        cli_t* client=initializeConn();
        if(pthread_create(&tid, NULL, child, (void *)client)<0)
         { perror("Unable to create new thread");
       exit(errorCode--);
     }
   }
   if (close(fd) == -1)
   {
    perror("Unable to close the file");
    exit(errorCode--);
  }
  if (close(connfd) == -1)
  {
    perror("Unable to close the socket");
    exit(errorCode--);
  }
}
