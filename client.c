#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <termios.h>

#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL IPPROTO_TCP
#define BUFFSIZE 256
#define SHMPERM 0600

int sfd;

void DieWithError(char *message);
void communication();

int main(int argc, char *argv[])
{
	unsigned short server_port;
	char *server_ip;
	int ID;
	struct sockaddr_in saddr;

	if(argc != 3)
		DieWithError("Usage: ./client <server_ip> <server_ip>\n");

	server_ip=argv[1];
	server_port=atoi(argv[2]);

	if((sfd=socket(DOMAIN, TYPE, PROTOCOL)) < 0)
		DieWithError("client-socket() failed\n");

	/* --- Local address structure --- */
	memset(&saddr, 0, sizeof(saddr));			/* Zero out structure */
	saddr.sin_family=DOMAIN;					/* Family -> AF_INET: Internet address */
	saddr.sin_port=htons(server_port);			/* Local port on which the server listen */
	saddr.sin_addr.s_addr=inet_addr(server_ip);	/* Any incoming interface */

	if((connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) < 0)
		DieWithError("connect() failed\n");

	for(;;)										/* Infinite loop which keep calling the same function */
	{
		communication();						/* Used to receive and then send something to the client */
	}

	close(sfd); 								/* Close the connection and destroy the socket */
	exit(0);
}

/* 
	Print an error message and exits.
*/
void DieWithError(char *message)
{
	perror(message);
	exit(1);
}

/*
	Function used to receive and then send something to the server.
	I also check if the answer provided by the server makes it exit.
*/
void communication()
{
	int bytereceived=0;
	char response[1000], input[BUFFSIZE];

	memset(response, 0, sizeof(response));
	if((bytereceived=recv(sfd, response, sizeof(response)-1, 0)) <= 0)
		DieWithError("recv() failed\n");

	response[bytereceived]='\0';
	fprintf(stdout, "%s", response);

	/* Checks during login if I failed it, if yes, exit */
	if((strcmp(response, "Login Failed!") == 0) || (strcmp(response, "Exiting the program\nBYE!") == 0))
	{
		close(sfd);
		exit(1);
	}

	fgets(input, BUFFSIZE, stdin);

	if((send(sfd, input, sizeof(input), 0)) < 0)
		DieWithError("send() failed\n");

	return ;
}