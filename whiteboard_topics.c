#include "whiteboard.h"

/*
	Load all the topics stored in the backup file in an array of structs for the topics.
*/
int load_topics()
{
	FILE *fd;
	struct stat st;
	char tmp[ANSSIZE], username[AUTHLEN], name[AUTHLEN];
	int size;

	if((fd=fopen(TOPICSDB, "r")) < 0)
		DieWithError("open() in load_topics() failed\n");
	
	if(stat(TOPICSDB, &st) == 0)	/* Get the total length of the file */
		size=st.st_size;

	/* Read the file of the topics saved and load them in an array of struct for the topics */
	if(size>1)
		while((fscanf(fd, "%s %s %s", tmp, username, name)) > 0)
		{
			id_counter[TOPICCOUNTER] = strtol(tmp, NULL, 0);
			topic[id_counter[TOPICCOUNTER]].topicid=id_counter[TOPICCOUNTER];
			strcpy(topic[id_counter[TOPICCOUNTER]].creator, username);
			strcpy(topic[id_counter[TOPICCOUNTER]].name, name);
		}
	/*else
	{
		id_counter[TOPICCOUNTER]=0;
		fclose(fd);
		return 0;
	}*/			// mezzo problema, il mio primo topic(quando parto da file vuoto) e' sempre al posto 1, non 0 ma l'id e' giusto, ovvero parte da 1

	
	id_counter[TOPICCOUNTER]+=1;
	
	fclose(fd);

	return 0;
}

int write_topics()
{
	FILE *fd;
	
	if((fd=fopen(TOPICSDB, "w")) < 0)		/* Open the file and overwrite the content with the new one */
		DieWithError("open() in write_topics() failed\n");
	
	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
	{
		if(topic[j].topicid > 0)	/* Check that the topics exists - if not, the id is = -1 */
			fprintf(fd, "%d %s %s\n", topic[j].topicid, topic[j].creator, topic[j].name); 
	}

	fclose(fd);

	return 0;
}
/*
	Function used to create a topic, asking the client for the fields needed to complete the elements in the struct.
		Uses the function pong() defined in utils.c to send and receive the proper fields.
*/
int create_topics(int client_socket, int current_id)	// NOTE: SE CREO IL TOPIC MI CI AUTO SUBSCRIBO
{
	//FILE *fd;
	int namelen, contentlen;

	strcpy(topic[id_counter[TOPICCOUNTER]].name, ping(client_socket, "### TOPIC CREATION MENU ###\nInsert the topic name: ", AUTHLEN));
	strcpy(topic[id_counter[TOPICCOUNTER]].creator, user[current_id].username);
	topic[id_counter[TOPICCOUNTER]].topicid=id_counter[TOPICCOUNTER];
	
	/*Remove the '\n' from the user's input fields */
	namelen=strlen(topic[id_counter[TOPICCOUNTER]].name);
	topic[id_counter[TOPICCOUNTER]].name[namelen-1]=0;

	//if((fd=fopen(TOPICSDB, "a+")) < 0)
	//	DieWithError("open() failed\n");
	id_counter[TOPICCOUNTER]+=1;
	
	ping(client_socket, "Topic created!\nPress ENTER to continue", ANSSIZE);

	/* Write in the file the fields of the new topic */
	//fprintf(fd, "%d %s %s %s\n", id_counter[TOPICCOUNTER], user[current_id].username, topic->name, topic->content); //asctime(tm) to print the current time
	//id_counter[TOPICCOUNTER]+=1;
	//fclose(fd);

	return 0;
}

//
//	RICORDA DI METTERE I SEMAFORI
//	ANCHE PER LE NUOVE FUNZIONALITA'
//

/*
	Function that lists the content of the file in which the topics are stored.
*/
int list_topics(int client_socket, int current_id)
{
	char *res;
	struct stat st;
	int namelen, contentlen, size;
	/* Write in the file the fields of the new topic */
	//fprintf(fd, "%d %s %s %s\n", id_counter[TOPICCOUNTER], user[current_id].username, topic->name, topic->content); //asctime(tm) to print the current time
	//id_counter[TOPICCOUNTER]+=1;
	//fclose(fd);
	// !!!! SE LISTO QUANDO NON HO NIENTE SI SBRAGA !!!! //
	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
	{
		if(topic[j].topicid > 0)	/* Check that the topics exists - if not, the id is = -1 */
		{
			for(int i=0; i<MAXSUBS; i++)
				if(user[current_id].topics_sub[i] == topic[j].topicid)
				{
					size=asprintf(&res, "ID: %d\tStatus: SUBSCRIBED\nCreator: %s\nTopic Name: %s\n\n", topic[j].topicid, topic[j].creator, topic[j].name);
					break;
				}
				else if(user[current_id].topics_sub[i] != topic[j].topicid && i == MAXSUBS-1)
					size=asprintf(&res, "ID: %d\tStatus: NOT SUBSCRIBED\nCreator: %s\nTopic Name: %s\n\n", topic[j].topicid, topic[j].creator, topic[j].name);
			
			send(client_socket, res, size, 0);
		}
	}
	
	ping(client_socket, "Press ENTER to continue", ANSSIZE);

	free(res);

	return 0;
}

/*
	The client choose the ID of a topic and then the server will delete it only if the client
		is also the owner of the topic.
*/
int delete_topic(int client_socket, int current_id)
{
	char id_char[ANSSIZE],
	res[] = "Topic deleted!\n";
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to DELETE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);

	if(strcmp(user[current_id].username, topic[id].creator) == 0)		/* If I am the owner of this topic */
	{	
		for(int j=0; j<id_counter[THREADCOUNTER]; j++)
			if(thread[j].topicid == id)
			{
				for(int i=0; i<id_counter[MSGCOUNTER]; i++)
					if(thread[j].threadid == message[i].threadid)
					{
						memset(message[i].creator, 0, sizeof(message[i].creator));
						memset(message[i].content, 0, sizeof(message[i].content));
						message[i].threadid=-1;
					}

				memset(thread[j].name, 0, sizeof(thread[j].name));
				memset(thread[j].creator, 0, sizeof(thread[j].creator));
				memset(thread[j].content, 0, sizeof(thread[j].content));
				thread[j].topicid=-1;
				thread[j].threadid=-1;
			}

		memset(topic[id].creator, 0, sizeof(topic[id].creator));
		memset(topic[id].name, 0, sizeof(topic[id].name));
		topic[id].topicid=-1;

		ping(client_socket, "Topic deleted!\nPress ENTER to continue", ANSSIZE);
	}
	else if(id >= id_counter[TOPICCOUNTER] || topic[id].topicid <= 0)
		ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
	else    /* Tell the client that he is not the owner of the topics he's trying to delete */
		ping(client_socket, "You're not the owner of this topic!\nPress ENTER to continue", ANSSIZE);
	
	//TOGLIERE L'ISCRIZIONE AGLI USERS ISCRITTI AD UN TOPIC CANCELLATO

	return 0;
}

/*
	The client choose the ID of a topic and tries to subscribe to it.
		The check the program perform are: 
			1) if the ID is related to an existing topic;
			2) if the client is already subscribed to the topic chosen;
			3) if the client has already reached the max number of topics he can subscribe to.
*/
int subscribe(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to SUBSCRIBE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)	/* Check that the ID of the topic exists */
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1))
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return 0;
		}
		else if(topic[j].topicid == id)
		{
			for(int j=0; j<MAXSUBS; j++)
				if(user[current_id].topics_sub[j] == id)	/* Check if I'm not already subscribed to the topic chosen */
				{
					ping(client_socket, "You are already subscribed to this topic!\nPress ENTER to continue!", ANSSIZE);
					return 0;
				}

			for(int j=0; j<MAXSUBS; j++)
				if(user[current_id].topics_sub[j] == 0)	/* At the first occurrence equal to 0 add the new topic */
				{
					user[current_id].topics_sub[j] = id;
					ping(client_socket, "Suibscription completed!\nPress ENTER to continue", ANSSIZE);
					return 0;
				}
				else if(user[current_id].topics_sub[j] > 0 && j == MAXSUBS-1)	/* Otherwise, if all the array is scanned and there are no spaces available, exit */
					ping(client_socket, "You have reached the limit of subscriptions!\nPress ENTER to continue!", ANSSIZE);
		}
	
	return 0;
}

/*
	Remove the subscription at a certain topic for the current user.
*/
void unsubscribe(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to SUBSCRIBE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1))
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return;
		}
		else if(topic[j].topicid == id)
		{
			for(int j=0; j<MAXSUBS; j++)
				if(user[current_id].topics_sub[j] == id)
				{
					user[current_id].topics_sub[j]=0;
					ping(client_socket, "Successfully unsubscribed\nPress ENTER to continue!", ANSSIZE);
					return;
				}
				else if(user[current_id].topics_sub[j] != id && j == MAXSUBS-1)
				{
					ping(client_socket, "You are not subscribed at this topic\nPress ENTER to continue!", ANSSIZE);
					return;
				}
		}

}