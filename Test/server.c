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

#include "netio.h"

#define SERVER_PORT 5678
int errorCode = -1;
int sockfd, connfd;
char buf[1024];
int fd, n, l, status;
struct sockaddr_in local_addr, rmt_addr;
int nread;
char s[10];
socklen_t rlen;
pid_t pid;

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
  rlen = sizeof(rmt_addr);
}

void openFile()
{
  fd = open("socketDescriptors.txt", O_RDWR | O_APPEND | O_TRUNC);
  if (fd == -1)
  {
    perror("File not found");
    exit(errorCode--);
  }
}

void initializeConn()
{
  if ((connfd = accept(sockfd, (struct sockaddr *)&rmt_addr, &rlen)) == -1)
  {
    perror("Unable to accept a conection on this socket");
    exit(errorCode--);
  }
  snprintf(s, sizeof(int), "%d", connfd);
  if (write(fd, s, strlen(s)) == -1)
  {
    perror("Unable to write the descriptor in the file");
    exit(errorCode--);
  }
}

int option = 0;
char credentials[256];
char name[100], pass[100];
void get_credentials()
{
  check_error(recv(connfd, credentials, 255, 0), "receive credentials");
  sscanf(credentials, "%d;%[^;];%[^;];", &option, name, pass);
}

void login()
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
  get_credentials();
  printf("daca merge... %d %s %s\n", option, name, pass);
  if (option == 1)
  {
    fprintf(users_fd, "%s;%s;\n", name, pass);
    strcpy(res, "1");
    check_error(send(connfd, res, strlen(res), 0), "Can't send response");
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
          check_error(send(connfd, res, strlen(res), 0), "Can't send response");
          break;
        }
      }
      strcpy(res, "0");
      check_error(send(connfd, res, strlen(res), 0), "Can't send response");
      get_credentials();
    }
    fseek(users_fd, 0, SEEK_SET);
  }
  fclose(users_fd);
}

void child()
{
  login();
  if (close(sockfd) == -1)
  {
    perror("Unable to close the socket");
    exit(errorCode--);
  }
  while (1)
  {
    nread = read(connfd, (void *)buf, 1024);
    if (nread <= 0)
    {
      perror("Unable to receive information from client");
      exit(errorCode--);
    }
    buf[nread] = '\0';
    printf("%s\n", buf);
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
  exit(0);
}

int main(void)
{
  prepareToReceiveRequests();
  openFile();
  for (;;)
  {
    initializeConn();
    pid = fork();
    if (pid == -1)
    {
      perror("Unable to create a child process");
      exit(errorCode--);
    }
    if (pid == 0)
    {
      child();
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
