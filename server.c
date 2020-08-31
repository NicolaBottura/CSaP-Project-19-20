#include "whiteboard.h"

int serve_the_client();

int main(int argc, char *argv[])
{
	unsigned short port;					/* Port on which the Server listen */
	int semvals[NUMSEM];					/* Array to manage semaphores */
	pid_t pid;								/* Variable to store return value of fork() */

	if(argc != 2)
		DieWithError("Usage: ./server <listening-port>\n");

	if(signal(SIGINT, sigint) < 0)			/* If ctrl+c while running it calls an handler defined in utils.c */
		DieWithError("signal() failed\n");

	semvals[SEMAUTH] = 1;					/* Set the value for the sem used to authentication */
	semvals[SEMTOPICS] = 1;					/* Set the value for the sem used for topic/threads/msgs' related operations */

	if(get_sem(SEMPERM | IPC_CREAT) < 0)	/* Call at the function that creates the semaphores */
		DieWithError("get_sem() failed\n");
	
	if(init_sem(semvals) < 0)				/* Call at the function that initialize the sempahores */
		DieWithError("init_sem() failed\n");

	if(init_shm(SHMPERM | IPC_CREAT) < 0)	/* Initialize the shared memory portion */
		DieWithError("init_shm() failed\n");

	port=atoi(argv[1]);						/* Get the port number passed as argument */
	server_socket=create_socket(port); 		/* Create the socket for communiations with clients */

	load_users();							/* Load all the users from file */
	load_utils();							/* Load utilities - subscriptions/unread msgs - from file */	
	load_topics();							/* Load all the topics from file */
	load_threads();							/* Load all the threads from file */
	load_messages();						/* Load all the messagtes from file */

	for(;;)												/* Run forever */
	{
		client_socket=accept_connection(server_socket);	/* Call at the function that accept connections from clients */
		
		switch(pid=fork())								/* Fork a new (child) process */
		{
			case 0:										/* Child process */
			{
				/* The authentication is blocking in order to make the clients authenticate only one at time */
				p(SEMAUTH);								/* sem-1 for AUTH */
				authentication(client_socket); 			/* Send the authentication form to the client */
				v(SEMAUTH);								/* sem+1 for AUTH */

				serve_the_client();						/* Print the MENU and manage the operations called by the clients */

				exit(0);								/* "Kill" the child spawned before */
			}
			case -1:
				DieWithError("fork() failed\n");
		}

		close(client_socket);							/* After the child exit, close its socket */
	}
	
	return 0;
}

/*
	Function that sends to the clients the MENU containing the list of possible operations
		and then, with a switch, manages all the functionalities.
	When a case ends, a recursive call to this function is made in order to send again the
		MENU to the client until it exits from the program.
*/
int serve_the_client()
{
	int operation=0,									/* Operation that the client will chose from the MENU */
	current_id=0;										/* ID of the client that's executing the operations in this moment */;
	char op[ANSSIZE];									/* Buffer used as return value of ping() function */

	strcpy(op, ping(client_socket, MENU, ANSSIZE));		/* Send the MENU to the client and receive the choosen operation */
	operation = strtol(op, NULL, 0); 					/* Convert the client's answer in integer */

	switch(operation)									/* Switch-case based on the choice of the client */
	{
		case 1:											/* CREATE A NEW TOPIC*/
		{
 			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");
			
			p(SEMTOPICS);								/* sem-1 for TOPICS */
			create_topics(client_socket, current_id);	/* Create a new topic - LOCATION: whiteboard_topics.c */
			v(SEMTOPICS);								/* sem+1 for TOPICS */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 2:											/* LIST TOPICS */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			list_topics(client_socket, current_id);		/* list all the topics presents in the struct */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 3:											/* DELETE A TOPIC */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			p(SEMTOPICS);								
			delete_topic(client_socket, current_id);	
			v(SEMTOPICS);

			serve_the_client(); //NOTA: faccio senza passare client_socket perche' e' global
		}
		case 4:											/* Reply to a thread(write a message) */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			p(SEMTOPICS);								/* Can't write a new message, if someone is deleting the topic the message will remain with no topic's references */
			reply(client_socket, current_id);
			v(SEMTOPICS);

			serve_the_client();
		}
		case 5:											/* Append a new thread to a topic */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			p(SEMTOPICS);								/* If someone is deleting a topic I can't append a new thread or it will remain with no topic's references */
			append(client_socket, current_id);
			v(SEMTOPICS);

			serve_the_client();
		}
		case 6:											/* List all threads and related messages */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");
				
			display_topic_content(client_socket, current_id);

			serve_the_client();
		}
		case 7:											/* Subscribe to a topic */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			subscribe(client_socket, current_id);

			serve_the_client();
		}
		case 8:											/* Show unread messages */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			show_unread(client_socket, current_id);

			serve_the_client();
		}
		case 9:											/* Unsubscribe from a topic */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			unsubscribe(client_socket, current_id);

			serve_the_client();
		}
		case 10:
		{	
			char res[] = "Exiting the program\nBYE!";

			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			user[current_id].logged=0;

			send(client_socket, res, sizeof(res), 0);
			close(client_socket);
			exit(0);	
		}
		default:
		{
			ping(client_socket, "Invalid Option\nPress ENTER to continue", ANSSIZE);	// non funzia
			serve_the_client();
		}
	}
}