#include "whiteboard.h"

/*
	Load all the existing threads, from the file in which are stored.
	(threads.txt) File format: THREAD_ID TOPIC_ID Creator Name Content
*/
void load_threads()
{
	FILE *fd;
	char tmp1[ANSSIZE], 																/* Buffer used to store the ID of the thread taken from file*/
	tmp2[ANSSIZE], 																		/* Buffer used to store the ID of the topic taken from file */
	username[AUTHLEN], 																	/* Buffer used to store the username of the creator of the thread */
	name[AUTHLEN], 																		/* Buffer used to store the name of the thread */
	content[CONTENTLEN];																/* Buffer used to store the content of the thread */

	if((fd=fopen(THREADDB, "r")) < 0)
		DieWithError("open() in load_threads() failed\n");								/* Open the file in READ mode */

	while((fscanf(fd, "%s %s %s %s %[^\n]", tmp1, tmp2, username, name, content)) > 0)	/* Read the file and store the content */
	{
		id_counter[THREADCOUNTER] = strtol(tmp1, NULL, 0);								/* Set the counter for the treahds as the ID of the current thread */
		thread[id_counter[THREADCOUNTER]].threadid=id_counter[THREADCOUNTER];			/* And set the ID of this thread as the value of the counter */
		thread[id_counter[THREADCOUNTER]].topicid = strtol(tmp2, NULL, 0);				/* Get the ID of the topic */
		strcpy(thread[id_counter[THREADCOUNTER]].creator, username);					/* The owner of the thread */
		strcpy(thread[id_counter[THREADCOUNTER]].name, name);							/* Its name */
		strcpy(thread[id_counter[THREADCOUNTER]].content, content);						/* And the content */
	}

	id_counter[THREADCOUNTER]+=1;														/* At the end, increment by 1 the counter */

	fclose(fd);

	return ;
}

/*
	Function used to write in a file the content of the structs of threads.
	This will be called when the server exits.
*/
void write_threads()
{
	FILE *fd;
	
	if((fd=fopen(THREADDB, "w")) < 0)													/* Open the file in WRITE mode to overwrite the content of it */
		DieWithError("open() in write_threads() failed\n");
	
	for(int j=0; j<id_counter[THREADCOUNTER]; j++)										/* After the crash of some clients, if the sempahores for the topics are >1 so more than a client can do operations for them */
		for(int i=0; i<id_counter[TOPICCOUNTER]; i++)									/* For example, someone delete a topic while another client is appending a thread and after this the topic is deleted but the thread not */
			if(thread[j].topicid != topic[i].topicid && i == id_counter[TOPICCOUNTER]-1)/* Check, when the server exits, if there are threads "alone" and delete them with also the associated messages */
			{
				for(int y=0; y<id_counter[MSGCOUNTER]; y++)
					if(thread[j].threadid == message[y].threadid)	/* If the message belongs to the thread */
					{
						/* -- Remove all the messages -- */
						memset(message[y].creator, 0, sizeof(message[y].creator));
						memset(message[y].content, 0, sizeof(message[y].content));
						message[y].threadid=0;
					}				

				/* -- Then remove the thread -- */
				memset(thread[j].name, 0, sizeof(thread[j].name));					
				memset(thread[j].creator, 0, sizeof(thread[j].creator));
				memset(thread[j].content, 0, sizeof(thread[j].content));
				thread[j].topicid=0;
				thread[j].threadid=0;
			}
			else continue;

	for(int j=0; j<id_counter[THREADCOUNTER]; j++)
	{
		if(thread[j].threadid > 0)														/* Check that the topics exists - if not, the id is = 0 */
			fprintf(fd, "%d %d %s %s %s\n", thread[j].threadid, thread[j].topicid, thread[j].creator, thread[j].name, thread[j].content); 
	}

	fclose(fd);

	return ;
}

/*
	Function used to append a new message(thread content) to a new thread under a specific topic.
		Ask for the topic ID on which the client wants to append this thread, then the name and the content of it.
*/
void append(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id, namelen, contentlen;

	strcpy(id_char, ping(client_socket, "\nChoose the ID of the topic in which you want to append the thread: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);					

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)										/* Check that the ID of the topic exists */
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1) || id <= 0 || topic[id].topicid <= 0)
		{
			ping(client_socket, "\nThis topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return ;
		}
		else break;

	for(int j=0; j<MAXSUBS; j++)
		if(user[current_id].topics_sub[j] == id) 										/* Check if the current user is subscribed to the chosen topic */
			break;	
		else if(user[current_id].topics_sub[j] != id && j == MAXSUBS-1)					/* If not, can't append a new thread */
		{
			ping(client_socket, "\nYou are not subscribed to this topic!\nPress ENTER to continue", ANSSIZE);
			return ;
		}

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)										/* Check that the ID of the topic exists */
	{
		if(topic[j].topicid == id)														/* If yes, add the new thread */
		{
			thread[id_counter[THREADCOUNTER]].threadid=id_counter[THREADCOUNTER]; 
			thread[id_counter[THREADCOUNTER]].topicid=id;
			strcpy(thread[id_counter[THREADCOUNTER]].creator, user[current_id].username);
			strcpy(thread[id_counter[THREADCOUNTER]].name, ping(client_socket, "\nInsert the name of this thread: ", AUTHLEN));
			strcpy(thread[id_counter[THREADCOUNTER]].content, ping(client_socket, "Insert the content of this thread: ", CONTENTLEN));
	
			/*Remove the '\n' from the user's input fields */
			namelen=strlen(thread[id_counter[THREADCOUNTER]].name);
			thread[id_counter[THREADCOUNTER]].name[namelen-1]=0;
			contentlen=strlen(thread[id_counter[THREADCOUNTER]].content);
			thread[id_counter[THREADCOUNTER]].content[contentlen-1]=0;

			id_counter[THREADCOUNTER]+=1;												/* Increment the counter of the threads by 1 */

			ping(client_socket, "\nThread created!\nPress ENTER to continue", ANSSIZE);

			return ;
		}
	}
}