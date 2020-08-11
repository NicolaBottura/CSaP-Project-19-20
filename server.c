#include "whiteboard.h"

int main(int argc, char *argv[])
{
	unsigned short port;	/* Port on which the Server listen */
	int child_counter=0,	/* Child processes counter */
	operation;
	pid_t pid;				/* Variable to store return value of fork() */
	char response[BUFFSIZE];

	if(signal(SIGINT, sigint) < 0)			/* If ctrl+c while running it calls an handler defined in utils.c*/
		DieWithError("signal() failed\n");

	if(get_sem(SEMPERM | IPC_CREAT) < 0)
		DieWithError("get_sem() failed\n");

	if(init_sem() < 0)
		DieWithError("init_sem() failed\n");

	if(init_shm(SHMPERM | IPC_CREAT) < 0)	/* Init the shared memory portion */
		DieWithError("init_shm() failed\n");

	port=atoi(argv[1]);					/* Get the port passed we the server program is launched */
	server_socket=create_socket(port); 	/* Create the socket for communiations with clients */

	for(;;)	/* Run forever */
	{
		client_socket=accept_connection(server_socket);
		

		switch(pid=fork())
		{
			case 0:		/* CHILD */
				p(0);

				authentication(client_socket); /* Send the authentication form and if the login is successful, send the menu */

				operation = strtol(buff, NULL, 0); /* Convert the client's answer in integer, which is stored in buff, the last
												item seen by the pong() function, which is global and definer in whiteboard.h */

				v(0);

				switch(operation)	/* Switch-case based on the choice of the client */
				{
					case 1:
					{
						create_topics(client_socket);
						break;
					}
					case 2:
					{
						list_topics(client_socket);
						break;
					}
				}

				//user->logged=0; /* Re-initialize the variable at 0 to simulate the logout */
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