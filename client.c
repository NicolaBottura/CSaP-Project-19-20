#include "whiteboard.h"

void DieWithError(char *message);

int main(int argc, char *argv[])
{
	unsigned short server_port;
	char *server_ip;
	int sfd;
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

	close(sfd); /* Close the connection and destroy the socket */
	exit(0);
}