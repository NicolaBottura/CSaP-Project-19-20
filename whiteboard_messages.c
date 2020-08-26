#include "whiteboard.h"

int load_messages()
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
	if(size>1)
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

	return 0;
}

int write_messages()
{
	FILE *fd;
	
	if((fd=fopen(MSGDB, "w")) < 0)		/* Open the file and overwrite the content with the new one */
		DieWithError("open() in write_messages() failed\n");
	
	for(int j=0; j<id_counter[MSGCOUNTER]; j++)
		if(message[j].threadid > 0)
			fprintf(fd, "%d %d %s %s\n", message[j].msgid, message[j].threadid, message[j].creator, message[j].content); 

	fclose(fd);

	return 0;
}

/* 
	Append a new message to a thread.
		Ask the client which thread he wants to reply to(asking the ID) and
			create a new instance of an array of struct of messages with inside the ID of the thread.
*/
int reply(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id, contentlen;

	// IPMPORTANTE -> IL TOPIC NUMERO 0 ESISTE ANCHE SE NON C'E' NIENTE DENTRO E POSSO QUINDI FARE LA REPLY, FIXARE STA COSA ASAP 

	strcpy(id_char, ping(client_socket, "Choose the thread ID on which you want to REPLY: ", ANSSIZE));		/* Ask the ID of the topic */
	id=strtol(id_char, NULL, 0);																			/* Convert it in an int type */

	for(int j=0; j<id_counter[THREADCOUNTER]; j++)	/* Check that the ID of the topic exists */
	{
		if(id >= id_counter[THREADCOUNTER] || (thread[j].threadid != id && j==id_counter[THREADCOUNTER]-1))
		{
			ping(client_socket, "This thread does not exist!\nPress ENTER to continue!", ANSSIZE);
			return 0;
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
				for(int x=0; x<MAXUNREAD; x++)
					if(user[i].usrid != current_id && user[i].unread_msg[x] == 0)
					{
						user[i].unread_msg[x]=id_counter[MSGCOUNTER];
						break;
					}

			id_counter[MSGCOUNTER]+=1;

			return 0;
		}
	}
}

void show_unread(int client_socket, int current_id)	/* di un topic a cui sono iscritto pero'! */
{
	int counter=0, size1, size2, operation, pos;
	char *tmp1, *tmp2, op[ANSSIZE];
	char status_menu [] = "Choose if you want to display:\n \
	1) All the messages unread\n \
	2) A specific message\n \
	0) Exit\n";

	for(int j=0; j<MAXUNREAD; j++)
		if(user[current_id].unread_msg[j] > 0)
			counter+=1;

	size1=asprintf(&tmp1, "You have %d messages unread!\n", counter);	// NOTA: da usare quando mi autentico: Benvenuto X, hai N messaggi non letti!
	send(client_socket, tmp1, size1, 0);

	for(int j=0; j<MAXUNREAD; j++)
		if(user[current_id].unread_msg[j] > 0)
		{		
			pos=user[current_id].unread_msg[j]; 
			size2=asprintf(&tmp2, "\nUNREAD message: %d\tFrom: %s\n\n", user[current_id].unread_msg[j], message[pos].creator); // aggiungi anche il topic?
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
				
		}
		case 0:
			return;
	}

	return;
}

/*
	Function that prints on the client's console all the threads and the related messages based on the topic.
*/
int list_messages(int client_socket)	/* trasforma in show topic # - se sono iscritto a quel topic */
{
	char *res1, *res2;
	struct stat st;
	int namelen, contentlen, size1, id, size2;

	// !!!! SE LISTO QUANDO NON HO NIENTE SI SBRAGA !!!! //
	for(int j=0; j<id_counter[THREADCOUNTER]; j++)
	{
		if(thread[j].threadid > 0)	/* Check that the topics exists - if not, the id is = -1 */
		{
			id=gettopicid(thread[j].topicid);

			size1=asprintf(&res1, "\nID: %d\tName: %s\tFrom: %s\tTopic: %s\nContent: %s\n", thread[j].threadid, thread[j].name, thread[j].creator, topic[id].name, thread[j].content);
			send(client_socket, res1, size1, 0);

			for(int i=0; i<id_counter[MSGCOUNTER]; i++)
				if(message[i].threadid == thread[j].threadid)
				{
					size2=asprintf(&res2, "\tID: %d\t%s: %s\n\n", message[i].msgid, message[i].creator, message[i].content);
					send(client_socket, res2, size2, 0);
					free(res2);
				}

			free(res1);
		}
	}
	
	ping(client_socket, "Press ENTER to continue", ANSSIZE);
}

/* 
	Get the ID of a certain topic based on the id of the thread passed in input.
*/
int gettopicid(int id)
{
	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
		if(id == topic[j].topicid)
			return topic[j].topicid;
}