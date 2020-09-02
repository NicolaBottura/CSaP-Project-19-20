#include "whiteboard.h"

/*
	Load all the existing topics, from the file in which are stored.
	(topics.txt) File format: ID Creator Content
*/
void load_topics()
{
	FILE *fd;
	char tmp[ANSSIZE], 														/* Buffer of size 3 used to store the ID taken from file  */
	username[AUTHLEN], 														/* Buffer used to store the username of the owner taken from file */
	name[AUTHLEN];															/* Buffer used to store the name of the topic taken from file */

	if((fd=fopen(TOPICSDB, "r")) < 0)										/* Open the file in READ mode */
		DieWithError("open() in load_topics() failed\n");
	
	while((fscanf(fd, "%s %s %[^\n]", tmp, username, name)) > 0)			/* Read the file and store the content */
	{
		id_counter[TOPICCOUNTER] = strtol(tmp, NULL, 0);					/* Convert the ID from char to int and update the counter for topics */
		topic[id_counter[TOPICCOUNTER]].topicid=id_counter[TOPICCOUNTER];	/* Set the topicid as the value of the counter */
		strcpy(topic[id_counter[TOPICCOUNTER]].creator, username);			/* Add the creator */
		strcpy(topic[id_counter[TOPICCOUNTER]].name, name);					/* and then the name of the topic */
	}

	id_counter[TOPICCOUNTER]+=1;											/* Increment the topic counter - if there are no topics in file it will start from 1, so
																					in the program I'll always check that the ID of the topic is > 0 */
	fclose(fd);

	return ;
}

/* 
	Function used to write in the file the topics that are present in the struct.
	This will be called when the server exits.
*/
void write_topics()
{
	FILE *fd;
	
	if((fd=fopen(TOPICSDB, "w")) < 0)										/* Open the file in WRITE mode to overwrite the content of it */
		DieWithError("open() in write_topics() failed\n");
	
	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
	{
		if(topic[j].topicid > 0)											/* Check that the topics exists - if not, the id is = 0 */
			fprintf(fd, "%d %s %s\n", topic[j].topicid, topic[j].creator, topic[j].name); 
	}

	fclose(fd);

	return ;
}
/*
	Function used to create a topic, asking the client for the fields needed to complete the elements in the struct.
	Uses the function ping() defined in utils.c to send and receive the proper fields.
	When I create a new topic, I also add the user that it's creating it in the subscribed list for this topic.
	If the number of subscriptions is reached, the user is asked if he wants to unsubscribe another topic or he wants 
		to continue without subscribing at the new one.
	This is done through a new menu that is printed once the event occurs.
*/
void create_topics(int client_socket, int current_id)
{
	int namelen, contentlen, op, pos;
	char subscribe_menu[] = "Max limit of subscription reached!\n \
	1) Unsubscribe from a topic to add the new one\n \
	2) Continue without subscribing\n",
	char_op[ANSSIZE];

	/* Same as always, ask the client for the topic name and add the fields in the struct with the ping() function, defined in utils.c */
	strcpy(topic[id_counter[TOPICCOUNTER]].name, ping(client_socket, "### TOPIC CREATION MENU ###\nInsert the topic name: ", AUTHLEN));
	strcpy(topic[id_counter[TOPICCOUNTER]].creator, user[current_id].username);
	topic[id_counter[TOPICCOUNTER]].topicid=id_counter[TOPICCOUNTER];
	
	/*Remove the '\n' from the user's input fields */
	namelen=strlen(topic[id_counter[TOPICCOUNTER]].name);
	topic[id_counter[TOPICCOUNTER]].name[namelen-1]=0;

	for(int j=0; j<MAXSUBS; j++)
		if(user[current_id].topics_sub[j] == 0)								/* If there is space for a new subscription */
		{
			user[current_id].topics_sub[j] = id_counter[TOPICCOUNTER];		/* add the ID of the topic in the subscription array */
			break;
		}
		else if(user[current_id].topics_sub[j] > 0 && j == MAXSUBS-1)		/* If MAXSUBS is reached */
		{
			strcpy(char_op, ping(client_socket, subscribe_menu, ANSSIZE));	/* send the new MENU to choose if continue without subscribing or delete the subscription to another topic */
			op=strtol(char_op, NULL, 0);

			switch(op)
			{
				case 1:														/* Remove the subscription to a topic and add the newly created */
				{
					if((pos=unsubscribe(client_socket, current_id)) >= 0)	/* Call the unsubscribe function used also in the normal flow of the program */
					{
						user[current_id].topics_sub[pos]=id_counter[TOPICCOUNTER];	/* Add the ID of the new topic in the array */
						break;
					}
					else
						break;
				}
				case 2:														/* Otherwise continue without subscribing */
				{
					break;
				}
			}
		}

	id_counter[TOPICCOUNTER]+=1;											/* Increment the counter of the topics by 1 after the creation */
	
	ping(client_socket, "Topic created!\nPress ENTER to continue", ANSSIZE);

	return ;
}

/*
	Function used to list all the existing topics.
	This function also check if the user is subscribed to a topic or not, and depending on the result it
		will print SUBSCRIBED or NOT SUBSCRIBED near the topic ID.
*/
void list_topics(int client_socket, int current_id)
{
	char *res;
	int size;

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
		if(topic[j].topicid == 0 && j == id_counter[TOPICCOUNTER]-1)		/* Check if there are topics stored, if not, send the proper message */
		{
			ping(client_socket, "No topics to show!\nPress ENTER to continue", ANSSIZE);
			return ;
		}

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
	{
		if(topic[j].topicid > 0)											/* Check that the topics exists - if not, the id is = 0 */
		{
			for(int i=0; i<MAXSUBS; i++)
				if(user[current_id].topics_sub[i] == topic[j].topicid)		/* If the user is subscribed to this topic */
				{
					size=asprintf(&res, "ID: %d\tStatus: SUBSCRIBED\nCreator: %s\nTopic Name: %s\n\n", topic[j].topicid, topic[j].creator, topic[j].name);
					break;
				}
				else if(user[current_id].topics_sub[i] != topic[j].topicid && i == MAXSUBS-1)
					size=asprintf(&res, "ID: %d\tStatus: NOT SUBSCRIBED\nCreator: %s\nTopic Name: %s\n\n", topic[j].topicid, topic[j].creator, topic[j].name);

			send(client_socket, res, size, 0);
			//ping(client_socket, res, ANSSIZE);
			free(res);
		}
	}

	ping(client_socket, "Press ENTER to continue", ANSSIZE);

	return ;
}

/*
	The client choose the ID of a topic and then the server will delete it.
	This can happen only if the client executing this operation is also the owner of the topic.
*/
void delete_topic(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to DELETE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0); 

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1) || id <= 0) /* Check if the topic chosen exists */
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else if(topic[j].topicid == id)
		{
			if(strcmp(user[current_id].username, topic[id].creator) == 0)	/* If I am the owner of this topic */
			{	
				for(int j=0; j<id_counter[AUTHCOUNTER]; j++)
					for(int i=0; i<MAXSUBS; i++)
						if(user[j].topics_sub[i] == id)						/* If the user has this topic in the subscription list, remove it */
							user[j].topics_sub[i] = 0;

				for(int z=0; z<id_counter[AUTHCOUNTER]; z++)				/* For each user */
					for(int i=0; i<MAXUNREAD; i++)							/* for each MAXUNREAD */
						for(int y=0; y<id_counter[THREADCOUNTER]; y++)		/* for each thread */
							for(int x=0; x<id_counter[MSGCOUNTER]; x++)		/* and for each message */
						/* If the thread belongs to the topic chosen and the message and, the user has this message in the unread array */
								if(thread[y].topicid == id && thread[y].threadid == message[x].threadid && user[z].unread_msg[i] == message[x].msgid)
									user[z].unread_msg[i] = 0;				/* Remove it from the array of unread messages */
								
				for(int j=0; j<id_counter[THREADCOUNTER]; j++)
					if(thread[j].topicid == id)								/* If the thread belongs to the topic */
					{
						for(int i=0; i<id_counter[MSGCOUNTER]; i++)
							if(thread[j].threadid == message[i].threadid)	/* If the message belongs to the thread */
							{
								/* -- Remove all the messages -- */
								memset(message[i].creator, 0, sizeof(message[i].creator));	
								memset(message[i].content, 0, sizeof(message[i].content));
								message[i].threadid=0;
							}
						/* -- Then remove the thread -- */
						memset(thread[j].name, 0, sizeof(thread[j].name));					
						memset(thread[j].creator, 0, sizeof(thread[j].creator));
						memset(thread[j].content, 0, sizeof(thread[j].content));
						thread[j].topicid=0;
						thread[j].threadid=0;
					}
				/* -- Finally, remove the topic -- */
				memset(topic[id].creator, 0, sizeof(topic[id].creator));					
				memset(topic[id].name, 0, sizeof(topic[id].name));
				topic[id].topicid=0;

				ping(client_socket, "Topic deleted!\nPress ENTER to continue", ANSSIZE);

				return ;
			}
			else    														/* Tell the client that he is not the owner of the topics he's trying to delete */
			{
				ping(client_socket, "You're not the owner of this topic!\nPress ENTER to continue", ANSSIZE);
				return ;
			}
		}

	return ;
}

/*
	The client choose the ID of a topic and tries to subscribe to it.
		The check the program perform are: 
			1) if the ID is related to an existing topic;
			2) if the client is already subscribed to the topic chosen;
			3) if the client has already reached the max number of topics he can subscribe to.
*/
void subscribe(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to SUBSCRIBE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)							/* Check that the chosen topic exists */
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1) || id <=0)
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else if(topic[j].topicid == id)										/* If yes */
		{
			for(int j=0; j<MAXSUBS; j++)
				if(user[current_id].topics_sub[j] == id)					/* Check if I'm not already subscribed to it */
				{
					ping(client_socket, "You are already subscribed to this topic!\nPress ENTER to continue!", ANSSIZE);
					return ;
				}

			for(int j=0; j<MAXSUBS; j++)
				if(user[current_id].topics_sub[j] == 0)						/* At the first occurrence equal to 0 add the new topic */
				{
					user[current_id].topics_sub[j] = id;
					ping(client_socket, "Subscription completed!\nPress ENTER to continue", ANSSIZE);
					return ;
				}
				else if(user[current_id].topics_sub[j] > 0 && j == MAXSUBS-1)	/* Otherwise, if all the array is scanned and there are no spaces available, exit */
				{	
					ping(client_socket, "You have reached the limit of subscriptions!\nPress ENTER to continue!", ANSSIZE);
					return ;
				}
		}
	
	return ;
}

/*
	Remove the subscription at a certain topic for the current user.
*/
int unsubscribe(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to UNSUBSCRIBE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)							/* Check that the chosen topic exists */
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1) || id <= 0)
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return -1;
		}
		else if(topic[j].topicid == id)										/* If yes */
		{
			for(int j=0; j<MAXSUBS; j++)
				if(user[current_id].topics_sub[j] == id)					/* and if I am also subscribed to it */
				{
					user[current_id].topics_sub[j]=0;						/* remove it from subscribed topics */
					ping(client_socket, "Successfully unsubscribed\nPress ENTER to continue!", ANSSIZE);
					return j;
				}
				else if(user[current_id].topics_sub[j] != id && j == MAXSUBS-1)
				{
					ping(client_socket, "You are not subscribed at this topic\nPress ENTER to continue!", ANSSIZE);
					return -1;
				}
		}
}