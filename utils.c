#include "whiteboard.h"

void DieWithError(char *message)
{
	perror(message);
	exit(1);
}

void sigint(int signal)
{
	/* Add the remove sem and shm function calls here */
	close(server_socket);
	close(client_socket);
	
	printf("Signal %d occurred, exiting.\n", signal);
	exit(0);
}

/* Function used to send a message back to the client through the socket 
	and get back the response by it.
	Client: ping
	Server: pong back
*/
char pong(int client_socket, char *message)
{
	char buff[BUFFSIZE];
	int msg_len, bytesreceived;

	msg_len = sizeof(message);

	if((send(client_socket, message, msg_len, 0)) != msg_len)
		DieWithError("send() failed\n");

	if((bytesreceived=recv(client_socket, buff, sizeof(buff), 0)) < 0)
		DieWithError("recv() failed\n");

	buff[bytesreceived]='\0';
	printf("Received: %s\n", buff);
	
	return buff;
}