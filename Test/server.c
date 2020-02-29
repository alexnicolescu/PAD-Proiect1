#include <stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<stdlib.h>

#include "netio.h"

#define SERVER_PORT 5678

int main(void){
	int sockfd,connfd;
	char buf[1024];
	struct sockaddr_in local_addr,rmt_addr;
	int nread;;
	socklen_t rlen;
	sockfd=socket(PF_INET,SOCK_STREAM,0);
	//se creaza un nou socket,primul argument e domeniul,in cazul nostru PF_INET(AF_INET) specifica folosirea IPv4.
	//al doilea e tipul socket-ului,in cazul nostru SOCK_STREAM indica o conexiune in doua cai
	//al treilea e tipul protocolului,de obicei se pune 0,iar sistemul de operare va alege protocolul potrivit,TCP pentru SOCK_STREAM,UDP pentru SOCK_DGRAM.
	set_addr(&local_addr,NULL,INADDR_ANY,SERVER_PORT);
	//Functia set_addr e folosita pentru completarea structurii sockaddr_in
	//primul argument va contine adresa si portul specificate de celelalte argumente
	//al doilea argument cel mai des se pune pe NULL
	//al treilea va fi INADDR_ANY(un socket cu aceasta adresa poate receptiona pachete si conexiuni de retea sosite catre oricare din adresele statiei )
	//ultimul argument reprezinta numarul de port,de regula un numar mai mare de 5000
	bind(sockfd,(struct sockaddr *)&local_addr,sizeof(local_addr));
	//Prin bind i se asociaza socket-ului o adresa IP si un numar de port.
	//primul argument e descriptorul socket-ului
	//al doilea reprezinta adresa pe care dorim sa o asociem socket-ului
	//al treilea e lungime in octeti a argumentului anterior
	listen(sockfd,5);
	//Prin listen anuntam faptul ca putem accepta conexiuni
	//primul argument e descriptorul socket-ului
	//al doilea e numarul maxim de cereri de stabilire a conexiuni

	rlen=sizeof(rmt_addr);
	connfd=accept(sockfd,(struct sockaddr *)&rmt_addr,&rlen);
	//Functia accept e folosita pentru a prelua o cerere de conexiune,creand un nou socket
	//noul socket e conectat la procesul client a carui cerere a fost preluata
	//primul argument e descriptorul socket-ului initial
	//al doilea argument va contine adresa si portul procesului client
	//al treilea argument e lungimea in octeti a argumentului anterior
	while((nread=stream_read(connfd,(void*)buf,1024))){
		buf[nread]='\0';
		printf("%s\n",buf);
	}
	if(nread<0)
	{
		printf("Eroare la citirea de la retea\n");
	}
	close(connfd);
	close(sockfd);
	exit(0);
}