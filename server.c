#include "whiteboard.h"

int serve_the_client();
int getcurrentid();

int main(int argc, char *argv[])
{
	unsigned short port;	/* Port on which the Server listen */
	int child_counter=0,	/* Child processes counter */
	semvals[NUMSEM];		/* Array to manage semaphores */
	pid_t pid;				/* Variable to store return value of fork() */

	if(argc != 2)
		DieWithError("Usage: ./server <listening-port>\n");

	if(signal(SIGINT, sigint) < 0)			/* If ctrl+c while running it calls an handler defined in utils.c*/
		DieWithError("signal() failed\n");

	semvals[SEMAUTH] = 1;
	semvals[SEMTOPICS] = 1;

	if(get_sem(SEMPERM | IPC_CREAT) < 0)
		DieWithError("get_sem() failed\n");
	
	if(init_sem(semvals) < 0)
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
				p(SEMAUTH);		/* sem-1 for AUTH */
				authentication(client_socket); /* Send the authentication form and if the login is successful, send the menu */
				v(SEMAUTH);		/* sem+1 for AUTH */
				
				serve_the_client();		/* Print the MENU and manage the operations called by the clients */

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

int serve_the_client()
{
	int operation=0,								/* Operation that the client will chose from the MENU */
	current_id=0;										/* ID of the client on which I want to do an operation */;
	char op[ANSSIZE];

	strcpy(op, pong(client_socket, MENU, ANSSIZE));		/* Send the MENU to the client */
	operation = strtol(op, NULL, 0); 					/* Convert the client's answer in integer */

	switch(operation)									/* Switch-case based on the choice of the client */
	{
		case 1:
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the client - needed in whiteboard_topics.c */
				DieWithError("getcurrentid() failed\n");
			
			p(SEMTOPICS);
			create_topics(client_socket, current_id);	/* Create a new topic - whiteboard_topics.c */
			v(SEMTOPICS);

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 2:
		{
			list_topics(client_socket);					/* list all the topics */	

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 0:
		{	
			pong(client_socket, "Exiting the program\nBYE!", 0);
			close(client_socket);
			exit(0);	
		}
	}
}

/* 
	Function that returns the ID of the client on which I want to do an operation.
		This is possible by checking the PID of the process that invokes this function.
*/
int getcurrentid()
{
	for(int j=0; j<*id_counter; j++){	
		if(user[j].pid == getpid())
			return user[j].usrid;
	}

	return -1;	
}