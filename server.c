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
				ping(client_socket, WELCOME, ANSSIZE);	/* Welcome string with the ASCII art */

				/* The authentication is blocking in order to make the clients authenticate only one at time */
				p(SEMAUTH);								/* sem-1 for AUTH */
				if(authentication(client_socket) < 0 )	/* Send the authentication form to the client */
					DieWithError("Login failed\n");
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

			p(SEMTOPICS);								/* sem-1 for TOPICS */
			delete_topic(client_socket, current_id);	/* Delete a topic from the structure and all the related threads and messages */
			v(SEMTOPICS);								/* sem+1 for TOPICS */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 4:											/* REPLY TO A THREAD */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			/* Can't write a new message, if someone is deleting the topic the message will remain with no topic's references */
			p(SEMTOPICS);								/* sem-1 for TOPICS */
			reply(client_socket, current_id);			/* Append a new message under an existing thread decided by the client */
			v(SEMTOPICS);								/* sem+1 for TOPICS */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 5:											/* CREATE A THREAD */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			/* If someone is deleting a topic I can't append a new thread or it will remain with no topic's references */
			p(SEMTOPICS);								/* sem-1 for TOPICS */
			append(client_socket, current_id);			/* Append a new thread under an existing topic decided by the client */
			v(SEMTOPICS);								/* sem+1 for TOPICS */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 6:											/* DISPLAY A TOPIC CONTENT */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");
				
			display_topic_content(client_socket, current_id);	/* Display all the threads and messages of a topic decided by the client */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 7:											/* SUBSCRIBE TO A TOPIC */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			subscribe(client_socket, current_id);		/* Add in the client calling this function the ID of the topic he want to subscribe to */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 8:											/* SHOW UNREAD MESSAGES */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			show_unread(client_socket, current_id);		/* Check if there are unread messages and then display them */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 9:											/* UNSUBSCRIBE A TOPIC */
		{
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			unsubscribe(client_socket, current_id);		/* Remove from the list of subscribed topics of the client calling this function a topic */

			serve_the_client();							/* Repeat this function to make the client execute another operation */
		}
		case 10:										/* QUIT */
		{	
			if((current_id=getcurrentid()) < 0)			/* Get the ID of the current client - LOCATION: utils.c */
				DieWithError("getcurrentid() failed\n");

			user[current_id].logged=0;					/* The client will result as not logged in */

			ping(client_socket, "Exiting the program\nBYE!", 0);
			close(client_socket);
			exit(1);
		}
		default:										/* If the user enter a number not between 1 and 10 or something else */
		{
			ping(client_socket, "Invalid Option\nPress ENTER to continue", ANSSIZE);
			serve_the_client();
		}
	}
}