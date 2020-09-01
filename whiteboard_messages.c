#include "whiteboard.h"

void load_messages()
{
	FILE *fd;
	struct stat st;
	int size;
	char tmp1[ANSSIZE], tmp2[ANSSIZE], content[CONTENTLEN], creator[AUTHLEN];

	if((fd=fopen(MSGDB, "r")) < 0)
		DieWithError("open() in load_messages() failed\n");
	
	if(stat(MSGDB, &st) == 0)	/* Get the total length of the file */
		size=st.st_size;

	/* Read the file of the topics saved and load them in an array of struct for the topics */
	while((fscanf(fd, "%s %s %s %[^\n]", tmp1, tmp2, creator, content)) > 0)
	{
		id_counter[MSGCOUNTER] = strtol(tmp1, NULL, 0);
		message[id_counter[MSGCOUNTER]].msgid=id_counter[MSGCOUNTER];
		message[id_counter[MSGCOUNTER]].threadid=strtol(tmp2, NULL, 0);	/* Get from the the file the ID of the topic */
		strcpy(message[id_counter[MSGCOUNTER]].creator, creator);
		strcpy(message[id_counter[MSGCOUNTER]].content, content);
	}

	id_counter[MSGCOUNTER]+=1;				/* +1 the number of messages */

	fclose(fd);

	return ;
}

void write_messages()
{
	FILE *fd;
	
	if((fd=fopen(MSGDB, "w")) < 0)		/* Open the file and overwrite the content with the new one */
		DieWithError("open() in write_messages() failed\n");
	
	for(int j=0; j<id_counter[MSGCOUNTER]; j++)
		if(message[j].threadid > 0)
			fprintf(fd, "%d %d %s %s\n", message[j].msgid, message[j].threadid, message[j].creator, message[j].content); 

	fclose(fd);

	return ;
}

/* 
	Append a new message to a thread.
		Ask the client which thread he wants to reply to(asking the ID) and
			create a new instance of an array of struct of messages with inside the ID of the thread.
*/
void reply(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id, contentlen;

	strcpy(id_char, ping(client_socket, "Choose the thread ID on which you want to REPLY: ", ANSSIZE));		/* Ask the ID of the thread */
	id=strtol(id_char, NULL, 0);																			/* Convert it in an int type */

	for(int j=0; j<MAXSUBS; j++)
		if(user[current_id].topics_sub[j] == thread[id].topicid) 	/* If I'm not suibscribed to the topic that belongs to the chosen thread, I can't write a new message under it */
			break;
		else if(user[current_id].topics_sub[j] != id && j == MAXSUBS-1)
		{
			ping(client_socket, "You are not subscribed to the topic of this thread!\nPress ENTER to continue", ANSSIZE);
			return ;
		}

	for(int j=0; j<id_counter[THREADCOUNTER]; j++)	/* Check that the ID of the topic exists */
	{
		if(id >= id_counter[THREADCOUNTER] || (thread[j].threadid != id && j==id_counter[THREADCOUNTER]-1) || id <= 0)
		{
			ping(client_socket, "This thread does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else if(thread[j].threadid == id)
		{
			message[id_counter[MSGCOUNTER]].msgid = id_counter[MSGCOUNTER];
			message[id_counter[MSGCOUNTER]].threadid = id;
			strcpy(message[id_counter[MSGCOUNTER]].creator, user[current_id].username);
			strcpy(message[id_counter[MSGCOUNTER]].content, ping(client_socket, "Insert the content of the message: ", CONTENTLEN));
	
			/*Remove the '\n' from the user's input fields */
			contentlen=strlen(message[id_counter[MSGCOUNTER]].content);
			message[id_counter[MSGCOUNTER]].content[contentlen-1]=0;

			for(int i=0; i<id_counter[AUTHCOUNTER]; i++)			/* Set the new message as UNREAD for every user different from the one who wrote it */
				for(int j=0; j<MAXSUBS; j++)
					if(user[i].topics_sub[j] == thread[id].topicid)	/* If the user is subscribed at this topic.. */
						for(int x=0; x<MAXUNREAD; x++)
							if(user[i].usrid != current_id && user[i].unread_msg[x] == 0)
							{
								user[i].unread_msg[x]=id_counter[MSGCOUNTER];	/* .. add the ID of the new message in the unread messages array */
								break;
							}

			id_counter[MSGCOUNTER]+=1;

			ping(client_socket, "Message added!\nPress ENTER to continue", ANSSIZE);

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
		which message I have not read from the topis I'm subscribed at - format: ID Creator
		Then, if there are messages unread, I can choose between two options:
			2.1) Display every unread message with their content - formnat: creator:content
			2.2) Display a specific message by entering its ID
*/
void show_unread(int client_socket, int current_id)
{
	int counter=0, size1, size2, operation, pos, id;
	char *tmp1, *tmp2, op[ANSSIZE], id_char[ANSSIZE];
	char status_menu [] = "Choose if you want to display:\n \
	1) All the messages unread\n \
	2) A specific message\n \
	0) Exit\n";

	for(int j=0; j<MAXUNREAD; j++)
		if(user[current_id].unread_msg[j] > 0)
			counter+=1;

	size1=asprintf(&tmp1, "\nYou have %d messages unread!\n", counter);	// NOTA: da usare quando mi autentico: Benvenuto X, hai N messaggi non letti!
	send(client_socket, tmp1, size1, 0);

	if(counter > 0)
	{
		for(int j=0; j<MAXUNREAD; j++)
			if(user[current_id].unread_msg[j] > 0)
			{		
				pos=user[current_id].unread_msg[j]; 
				size2=asprintf(&tmp2, "\nUNREAD message ID: %d\tFrom: %s\n\n", user[current_id].unread_msg[j], message[pos].creator); // aggiungi anche il topic?
				send(client_socket, tmp2, size2, 0);
			}

		free(tmp2);
	
		strcpy(op, ping(client_socket, status_menu, ANSSIZE));		/* Send the MENU to the client */
		operation = strtol(op, NULL, 0); 		

		switch(operation)
		{
			case 1:	/* Show all the unread messagess */
			{
				for(int j=0; j<MAXUNREAD; j++)
					if(user[current_id].unread_msg[j] > 0)
					{
						pos=user[current_id].unread_msg[j]; 
						size1=asprintf(&tmp1, "\nID: %d\t%s: %s\n\n", message[pos].msgid, message[pos].creator, message[pos].content);
						send(client_socket, tmp1, size1, 0);
						free(tmp1);
					}
				
				for(int j=0; j<MAXUNREAD; j++)
					if(user[current_id].unread_msg[j] > 0)
						user[current_id].unread_msg[j]=0;	/* Set to READ (=0) */

				break;
			}
			case 2: /* Choose the ID of the message you want to read and display it */
			{
				strcpy(id_char, ping(client_socket, "Choose the ID of the message you want to display: ", ANSSIZE));		/* Send the MENU to the client */
				id=strtol(id_char, NULL, 0);

				for(int j=0; j<id_counter[MSGCOUNTER]; j++)
					if(message[j].msgid == id)
					{
						for(int j=0; j<MAXUNREAD; j++)
							if(id == user[current_id].unread_msg[j])
							{
								size2=asprintf(&tmp2, "\n%s: %s\n\n", message[id].creator, message[id].content);
								send(client_socket, tmp2, size2, 0);

								user[current_id].unread_msg[j]=0;

								free(tmp2);
								return;							
							}
					}
					else if(message[j].msgid != id && j == id_counter[MSGCOUNTER]-1)
					{
						ping(client_socket, "This message does not exist!\nPress ENTER to continue!", ANSSIZE);
						return;
					}
			}
			case 0:
				break;
			default:
			{	
				ping(client_socket, "Invalid operation, exiting from here!\nPress ENTER to continue!", ANSSIZE);
				break;
			}
		}
	}

	return;
}

/*
	Function that prints on the client's console all the threads and the related messages based on the topic.
	Prints out all the threads from the topics to which I am subscribed and the related messages, read and unread, and if there are
	unread message I'll set them as read.
	Format:	Thread-ID Thread-name Thread-creator Topic-name Thread-Content
					Message-ID Message-creator:Message-Content
*/
void display_topic_content(int client_socket, int current_id)	/* mi serve perche' se mi iscrivo dopo ad un topic, non ho i messaggi nell'unread msg e quindi posso visualizzare comunque tutto */
{
	char *res1, *res2;				// IMPORTANTE, METTI TUTTI I MESSAGGI COME READ
	struct stat st;
	int namelen, contentlen, size1, id, size2;
	char id_char[ANSSIZE];

	strcpy(id_char, ping(client_socket, "Choose the ID of the TOPIC you want to see: ", ANSSIZE));		/* Ask the ID of the thread */
	id=strtol(id_char, NULL, 0);

	for(int j=0; j<id_counter[THREADCOUNTER]; j++)
		if(thread[j].threadid == 0 && j == id_counter[THREADCOUNTER]-1)	/* Check if there are topics stored */
		{
			ping(client_socket, "No threads to show!\nPress ENTER to continue", ANSSIZE);
			return ;
		}

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)	/* Check that the ID of the topic exists */
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1) || id <= 0)
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else if(topic[j].topicid == id)
		{
			for(int i=0; i<MAXSUBS; i++)
				if(user[current_id].topics_sub[i] == id)	/* If the user is subscribed at this topic.. */
				{
					for(int x=0; x<id_counter[THREADCOUNTER]; x++)
						if(thread[x].threadid > 0 && thread[x].topicid == id)	/* Check that the thread exists */
						{
							size1=asprintf(&res1, "\nID: %d\tName: %s\tFrom: %s\tTopic: %s\nContent: %s\n\n", thread[x].threadid, thread[x].name, thread[x].creator, topic[id].name, thread[x].content);
							send(client_socket, res1, size1, 0);
							free(res1);

							for(int y=0; y<id_counter[MSGCOUNTER]; y++)
								if(message[y].threadid == thread[x].threadid)
								{
									size2=asprintf(&res2, "\tID: %d\t%s: %s\n\n", message[y].msgid, message[y].creator, message[y].content);
									send(client_socket, res2, size2, 0);
									free(res2);

									for(int z=0; z<MAXUNREAD; z++)
										if(user[current_id].unread_msg[z] == message[y].msgid)
											user[current_id].unread_msg[z]=0;
								}
						}

					ping(client_socket, "Press ENTER to continue", ANSSIZE);
					return ;
				}
				else if(user[current_id].topics_sub[i] != id && i == MAXSUBS-1)
				{
					ping(client_socket, "You can't see topics you're not subscribed to!\nPress ENTER to continue", ANSSIZE);
					return ;
				}
			
			//return 0;	/* Without this here, it will execute another round in the for and print the topic not existing message */
		}	
}