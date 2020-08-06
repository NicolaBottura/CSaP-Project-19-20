#include "whiteboard.h"

int main(int argc, char *argv[])
{
	unsigned short server_port;
	char *server_ip, buff[BUFFSIZE], *op; // fixa
	int sfd, bytereceived;
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


	if((bytereceived=recv(sfd, buff, sizeof(buff), 0)) <= 0)
		DieWithError("recv() failed\n");

	buff[bytereceived]='\0';
	printf("%s\n", buff);

	scanf("%s", op);

	send(sfd, op, sizeof(op), 0);


	close(sfd); /* Close the connection and destroy the socket */
	exit(0);
}