#include "whiteboard.h"

int getcurrentid();

int main(int argc, char *argv[])
{
	unsigned short port;	/* Port on which the Server listen */
	int child_counter=0,	/* Child processes counter */
	operation,				/* Operation chosen froim the MENU by the client after authentication */
	current_id=0;			/* ID of the client on which I want to do an operation */
	pid_t pid;				/* Variable to store return value of fork() */

	if(signal(SIGINT, sigint) < 0)			/* If ctrl+c while running it calls an handler defined in utils.c*/
		DieWithError("signal() failed\n");

	if(get_sem(SEMPERM | IPC_CREAT) < 0)
		DieWithError("get_sem() failed\n");

	if(init_sem() < 0)
		DieWithError("init_sem() failed\n");

	if(init_shm(SHMPERM | IPC_CREAT) < 0)	/* Initialize the shared memory portion */
		DieWithError("init_shm() failed\n");

	port=atoi(argv[1]);						/* Get the port passed we the server program is launched */
	server_socket=create_socket(port); 		/* Create the socket for communiations with clients */

	*id_counter=0;							/* Set the value of the id_counter(which is global and in shm) to 0 */

	for(;;)	/* Run forever */
	{
		client_socket=accept_connection(server_socket);

		switch(pid=fork())
		{
			case 0:		/* CHILD */
			{
				p(0);		/* sem-1 */

				authentication(client_socket); /* Send the authentication form and if the login is successful, send the menu */

				operation = strtol(buff, NULL, 0); /* Convert the client's answer in integer, which is stored in buff, the last
												item seen by the pong() function, which is global and definer in whiteboard.h */

				v(0);		/* sem+1 */

				printf("id: %d\n", *id_counter);
				
				switch(operation)	/* Switch-case based on the choice of the client */
				{
					case 1:
					{
						current_id=getcurrentid();

						create_topics(client_socket, current_id);	/* Create a new topic */
						break;
					}
					case 2:
					{
						list_topics(client_socket);		/* list all the topics */
						break;
					}
				}

				//user->logged=0; /* Re-initialize the variable at 0 to simulate the logout */
				exit(0);
			}
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

/* 
	Function that returns the ID of the client on which I want to do an operation.
		This is possible by checking the PID of the process that invokes this function.
*/
int getcurrentid()
{
	for(int j=0; j<*id_counter; j++)		
		if(user[j].pid == getpid())	
			return user[j].usrid;
}