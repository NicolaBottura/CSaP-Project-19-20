#include "whiteboard.h"

void DieWithError(char *message);
void sigint(int signal);

int main(int argc, char *argv[])
{
	unsigned short port;	/* Port on which the Server listen */
	int child_counter=0;	/* Child processes counter */
	pid_t pid;				/* Variable to store return value of fork() */

	if(signal(SIGINT, sigint) < 0)			/* If ctrl+c while running it calls an handler defined in utils.c*/
		DieWithError("signal() failed\n");

	port=atoi(argv[1]);
	server_socket=create_socket(port);

	for(;;)	/* Run forever */
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
			if(pid < 0)									/* If error of waitpid() */
				DieWithError("waitpid() failed\n");
			else if(pid == 0)							/* No zombies to wait on */
				break;
			else
				child_counter--;						/* Reduce by 1 the number of childs once one is dead */
		}
	}

	return 0; /* Never reaches this section so I don't even write the close(server_sorcket) here */
}