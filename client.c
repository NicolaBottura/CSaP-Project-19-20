#include "whiteboard.h"

void communication(int sfd);

int main(int argc, char *argv[])
{
	unsigned short server_port;
	char *server_ip;
	int sfd;
	struct sockaddr_in saddr;

	if(argc != 3)
		DieWithError("Usage: ./client <server_ip> <server_ip>\n");

	if(init_shm(SHMPERM) < 0)					
		DieWithError("init_shm() failed\n");

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

	
	communication(sfd); /* Username exchange */
	communication(sfd); /* Password exchange */
	communication(sfd); /* Login Response exchange */
	
	close(sfd); /* Close the connection and destroy the socket */
	exit(0);
}

/* Function used to send and receive messages from the server */
void communication(int sfd)
{
	int bytereceived=0;
	char response[BUFFSIZE], input[BUFFSIZE];

	memset(response, 0, sizeof(response));
	if((bytereceived=recv(sfd, response, sizeof(response)-1, 0)) <= 0)
		DieWithError("recv() failed\n");

	response[bytereceived]='\0';
	printf("%s\n", response);

	/* Checks during login if I failed it, if yes, exit */
	if((strcmp(response, "Login Failed!") == 0) && (user->logged == 0))
	{
		close(sfd);
		exit(0);
	}

	scanf("%s", input);

	if((send(sfd, input, sizeof(input), 0)) < 0)
		DieWithError("send() failed\n");

	return ;
}