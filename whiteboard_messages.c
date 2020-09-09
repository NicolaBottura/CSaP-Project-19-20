#include "whiteboard.h"

/*
	Load all the existing messages, from the file in which are stored.
	(messages.txt) File format: MSG_ID THREAD_ID Creator Content
*/
void load_messages()
{
	FILE *fd;
	char tmp1[ANSSIZE],															/* Buffer used to store the ID of the message taken from file*/
	tmp2[ANSSIZE], 																/* Buffer used to store the ID of the thread taken from file*/
	content[CONTENTLEN], 														/* Buffer used to store the content of the message taken from file*/
	creator[AUTHLEN];															/* Buffer used to store the creator of the message taken from file*/

	if((fd=fopen(MSGDB, "r")) < 0)												/* Open the file in READ mode */
		DieWithError("open() in load_messages() failed\n");
	
	while((fscanf(fd, "%s %s %s %[^\n]", tmp1, tmp2, creator, content)) > 0)	/* Read the file and store the content */
	{
		id_counter[MSGCOUNTER] = strtol(tmp1, NULL, 0);							/* Set the counter of the messages as the ID of the current message */
		message[id_counter[MSGCOUNTER]].msgid=id_counter[MSGCOUNTER];			/* Set the ID of the message as the value of the counter */
		message[id_counter[MSGCOUNTER]].threadid=strtol(tmp2, NULL, 0);			/* Set the ID of the thread */
		strcpy(message[id_counter[MSGCOUNTER]].creator, creator);				/* Set the creator of the message */
		strcpy(message[id_counter[MSGCOUNTER]].content, content);				/* And then the content of it */
	}

	id_counter[MSGCOUNTER]+=1;													/* At the end, increment by 1 the counter */

	fclose(fd);

	return ;
}

/*
	Function used to write in a file the content of the structs of messages.
	This will be called when the server exits.
*/
void write_messages()
{
	FILE *fd;
	
	if((fd=fopen(MSGDB, "w")) < 0)												/* Open the file in WRITE mode to overwrite the content of it */
		DieWithError("open() in write_messages() failed\n");
	
	for(int j=0; j<id_counter[MSGCOUNTER]; j++)
		if(message[j].threadid > 0)												/* Check that the thread exists - if not, the id is = 0 */
			fprintf(fd, "%d %d %s %s\n", message[j].msgid, message[j].threadid, message[j].creator, message[j].content); 

	fclose(fd);

	return ;
}

/* 
	Add a new message to a thread.
		Ask the client which thread he wants to reply to(asking the ID) and
			create a new instance of an array of struct of messages with inside the ID of the thread as reference.
*/
void reply(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id, contentlen;

	strcpy(id_char, ping(client_socket, "\nChoose the thread ID on which you want to REPLY: ", ANSSIZE));	/* Ask the ID of the thread */
	id=strtol(id_char, NULL, 0);																			/* Convert it in an int type */

	for(int j=0; j<MAXSUBS; j++)
		if(user[current_id].topics_sub[j] == thread[id].topicid) 											/* Check if the current user is subscribed to the chosen topic */
			break;
		else if(user[current_id].topics_sub[j] != id && j == MAXSUBS-1)										/* If not, can't add a new message */
		{
			ping(client_socket, "\nYou are not subscribed to the topic of this thread!\nPress ENTER to continue", ANSSIZE);
			return ;
		}

	for(int j=0; j<id_counter[THREADCOUNTER]; j++)															/* Check that the ID of the thread exists */
	{
		if(id >= id_counter[THREADCOUNTER] || (thread[j].threadid != id && j==id_counter[THREADCOUNTER]-1) || id <= 0 || thread[id].threadid <= 0)
		{
			ping(client_socket, "\nThis thread does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else if(thread[j].threadid == id)																	/* If yes, add the new message */
		{
			message[id_counter[MSGCOUNTER]].msgid = id_counter[MSGCOUNTER];
			message[id_counter[MSGCOUNTER]].threadid = id;
			strcpy(message[id_counter[MSGCOUNTER]].creator, user[current_id].username);
			strcpy(message[id_counter[MSGCOUNTER]].content, ping(client_socket, "\nInsert the content of the message: ", CONTENTLEN));
	
			/*Remove the '\n' from the user's input fields */
			contentlen=strlen(message[id_counter[MSGCOUNTER]].content);
			message[id_counter[MSGCOUNTER]].content[contentlen-1]=0;

			/* At this points, I want to add the new message in the unread messages array for all the users that are subscribed to this topic */
			for(int i=0; i<id_counter[AUTHCOUNTER]; i++)
				for(int j=0; j<MAXSUBS; j++)
					if(user[i].topics_sub[j] == thread[id].topicid)											/* If the user is subscribed at this topic */
						for(int x=0; x<MAXUNREAD; x++)
							if(user[i].usrid != current_id && user[i].unread_msg[x] == 0)					/* And if the user has not this message in the unread ones and has space to add a new instance */
							{
								user[i].unread_msg[x]=id_counter[MSGCOUNTER];								/* Add the ID of the new message in the unread messages array */
								
								if(x+1 == MAXUNREAD)
									user[i].unread_msg[0]=0;
								else
									user[i].unread_msg[x+1]=0;
								
								break;
							}					

			id_counter[MSGCOUNTER]+=1;																		/* At the end, increment the counter by 1 */

			ping(client_socket, "\nMessage added!\nPress ENTER to continue", ANSSIZE);

			return ;
		}
	}
}

/*
	This function is composed by two client functionalities:
		1) ~status[message#]
		2) GET[message#]
	My interpretation of these commands were: 
		for the status, instead of showing the status of a specific message, I'll choose this option and the server show me
		which message I have not read from the topics I'm subscribed at, showing ID of the message and the Creator.
		Then, if there are messages unread, I can choose between two options:
			2.1) Display every unread message with their content - format: creator:content
			2.2) Or, display a specific message by entering its ID
*/
void show_unread(int client_socket, int current_id)
{
	int counter=0, size1, size2, operation, pos, id;
	char *tmp1, *tmp2, op[ANSSIZE], id_char[ANSSIZE];
	char status_menu [] = "\nUnread messages menu\n \
	1) All the messages unread\n \
	2) A specific message\n \
	0) Exit\n \
	choose an operation: ";

	for(int j=0; j<MAXUNREAD; j++)																			/* Check how many unread messages has the current user */
		if(user[current_id].unread_msg[j] > 0)
			counter+=1;

	size1=asprintf(&tmp1, "\nYou have %d messages unread!\n", counter);										/* Show to the user the total number of unread messages */
	send(client_socket, tmp1, size1, 0);

	if(counter > 0)
	{
		for(int j=0; j<MAXUNREAD; j++)
			if(user[current_id].unread_msg[j] > 0)															/* If I have unread messages show the ID, the writer and the thread of the message */
			{		
				pos=user[current_id].unread_msg[j]; 
				size2=asprintf(&tmp2, "\nUnread message ID: %d\tFrom: %s\tIn threadd: %s\n", user[current_id].unread_msg[j], message[pos].creator, thread[message[pos].threadid].name);
				send(client_socket, tmp2, size2, 0);
			}

		free(tmp2);
	
		strcpy(op, ping(client_socket, status_menu, ANSSIZE));												/* Send the MENU to the client asking for a new operation */
		operation = strtol(op, NULL, 0); 		

		switch(operation)
		{
			case 1:																							/* Show all the unread messagess */
			{
				for(int j=0; j<MAXUNREAD; j++)
					if(user[current_id].unread_msg[j] > 0)
					{
						pos=user[current_id].unread_msg[j]; 
						size1=asprintf(&tmp1, "\n\tID: %d\t%s: %s\n\n", message[pos].msgid, message[pos].creator, message[pos].content);
						send(client_socket, tmp1, size1, 0);
						free(tmp1);
					}
				
				for(int j=0; j<MAXUNREAD; j++)
					if(user[current_id].unread_msg[j] > 0)
						user[current_id].unread_msg[j]=0;													/* Set all the messages in the unread array as READ */

				break;
			}
			case 2: 																						/* Choose the ID of the message you want to read and display it */
			{
				strcpy(id_char, ping(client_socket, "\nChoose the ID of the message you want to display: ", ANSSIZE));
				id=strtol(id_char, NULL, 0);

				for(int j=0; j<id_counter[MSGCOUNTER]; j++)
					if(message[j].msgid != id && j == id_counter[MSGCOUNTER]-1 || id <= 0)					/* If the message does not exist, send the proper message */
					{
						ping(client_socket, "\nThis message does not exist!\nPress ENTER to continue!", ANSSIZE);
						return;
					}
					else if(message[j].msgid == id)															/* If the chosen message exists */
					{
						for(int j=0; j<MAXUNREAD; j++)
							if(id == user[current_id].unread_msg[j])										/* Check again that the user has this message in the unread array */
							{
								size2=asprintf(&tmp2, "\n\t%s: %s\n", message[id].creator, message[id].content);
								send(client_socket, tmp2, size2, 0);

								user[current_id].unread_msg[j]=0;

								free(tmp2);
								return;							
							}
					}		
			}
			case 0:
				break;
			default:
			{	
				ping(client_socket, "\nInvalid operation, exiting from here!\nPress ENTER to continue!", ANSSIZE);
				break;
			}
		}
	}

	return;
}

/*
	Function that prints on the client's console all the threads and the related messages of a topic chosen by the client.
	Prints out all the threads from the topics to which I am subscribed and the related messages, read and unread, and if there are
	unread message I'll set them as read.
	Format:	Thread-ID Thread-name Thread-creator Topic-name Thread-Content
					Message-ID Message-creator:Message-Content
*/
void display_topic_content(int client_socket, int current_id)
{
	char *res1, *res2;
	struct stat st;
	int namelen, contentlen, size1, id, size2;
	char id_char[ANSSIZE];

	strcpy(id_char, ping(client_socket, "\nChoose the ID of the TOPIC you want to see: ", ANSSIZE));		/* Ask the ID of the thread */
	id=strtol(id_char, NULL, 0);

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)															/* Check that the ID of the topic exists */
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1) || id <= 0)
		{
			ping(client_socket, "\nThis topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else if(topic[j].topicid == id)																		/* If yes */
		{
			for(int i=0; i<MAXSUBS; i++)
				if(user[current_id].topics_sub[i] == id)													/* If the user is subscribed at this topic */
				{
					for(int x=0; x<id_counter[THREADCOUNTER]; x++)											/* For each existing thread */
					{
						if(thread[x].threadid > 0 && thread[x].topicid == id)								/* Belonging to this topic */
						{
							/* Send the thread infos line */
							size1=asprintf(&res1, "\nID: %d\tName: %s\tFrom: %s\tTopic: %s\n\tContent: %s\n\n", thread[x].threadid, thread[x].name, thread[x].creator, topic[id].name, thread[x].content);
							send(client_socket, res1, size1, 0);
							free(res1);

							for(int y=0; y<id_counter[MSGCOUNTER]; y++)										/* Then, for each message */
								if(message[y].threadid == thread[x].threadid)								/* Belonging to this thread */
								{
									/* Send all the messages */
									size2=asprintf(&res2, "\tID: %d\t%s: %s\n", message[y].msgid, message[y].creator, message[y].content);
									send(client_socket, res2, size2, 0);
									free(res2);

									for(int z=0; z<MAXUNREAD; z++)
										if(user[current_id].unread_msg[z] == message[y].msgid)				/* And if these messages are in the unread array of the user, set them as READ */
											user[current_id].unread_msg[z]=0;
								}
						}
					}

					ping(client_socket, "\nPress ENTER to continue", ANSSIZE);
					return ;
				}
				else if(user[current_id].topics_sub[i] != id && i == MAXSUBS-1)
				{
					ping(client_socket, "\nYou can't see topics you're not subscribed to!\nPress ENTER to continue", ANSSIZE);
					return ;
				}
		}	
}