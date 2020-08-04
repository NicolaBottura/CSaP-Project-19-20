#include "whiteboard.h"

void DieWithError(char *message);
void sigint(int signal);

int main(int argc, char *argv[])
{
	unsigned short port;	/* Port on which the Server listen */
	int server_socket, 		/* Server Socket FD */
	client_socket,			/* Client Socket FD */
	child_counter=0;		/* Child processes counter */
	pid_t pid;				/* Variable I'll use as a return value for fork() */

	if(signal(SIGINT, sigint) < 0)
		DieWithError("signal() failed\n");

	port=atoi(argv[1]);
	server_socket=create_socket(port);

	for(;;)
	{
		client_socket=accept_connection(server_socket);

		switch(pid=fork())
		{
			case 0:		/* CHILD */
				exit(0);
			case -1:	/* Error */
				DieWithError("fork() failed\n");
		}

		close(client_socket);	/* After the child exit, close its socket */
		child_counter++;		/* Increment the number of pending child processes */

		while(child_counter)
		{
			pid = waitpid((pid_t) -1, NULL, WNOHANG);	/* Non-blocking wait */
			if(pid < 0)		/* If error of waitpid() */
				DieWithError("waitpid() failed\n");
			else if(pid == 0)	/* No zombies to wait on */
				break;
			else
				child_counter--;	/* Reduce by 1 the number of childs once one is dead */
		}
	}

	exit(0);
}