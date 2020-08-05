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