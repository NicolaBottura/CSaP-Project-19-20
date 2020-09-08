#include "whiteboard.h"

int create_socket(unsigned short port)
{
	int sfd;
	struct sockaddr_in saddr;

	if((sfd=socket(DOMAIN, TYPE, PROTOCOL)) < 0)					/* Create a socket for incoming connections */
		DieWithError("socket() failed\n");

	/* --- Local address structure --- */
	memset(&saddr, 0, sizeof(saddr));								/* Zero out structure */
	saddr.sin_family=DOMAIN;										/* Family -> AF_INET: Internet address */
	saddr.sin_port=htons(port);										/* Local port on which the server listen */
	saddr.sin_addr.s_addr=INADDR_ANY;								/* Any incoming interface */

	if((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) < 0)	/* Bind the socket created before to the local address */
		DieWithError("bind() failed\n");

	if((listen(sfd, MAXPENDING)) < 0)								/* Make the server listen on the socket accepting incoming connection, until it reaches the queue limit */
		DieWithError("listen() failed\n");

	return sfd;
}

int accept_connection(int server_socket)
{
	int client_socket;												/* Client Socket FD */
	struct sockaddr_in caddr;										/* Client Address */
	unsigned int len;												/* Length of client address data structure */

	len=sizeof(caddr);

	/* Extracts the first connection request on the queue of pending connections */
	if((client_socket=accept(server_socket, (struct sockaddr *)&caddr, &len)) < 0)
		DieWithError("accept() failed\n");

	printf("[ACCEPT_CONNECTION]: Handling client %s\n", inet_ntoa(caddr.sin_addr));

	return client_socket;
}