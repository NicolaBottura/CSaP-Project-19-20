#include "whiteboard.h"

/*
	Load all the topics stored in the backup file in an array of structs for the topics.
*/
int load_topics()
{
	FILE *fd;
	struct stat st;
	char tmp[ANSSIZE], username[AUTHLEN], name[AUTHLEN], content[CONTENTLEN];
	int size;

	if((fd=fopen(TOPICSDB, "r")) < 0)
		DieWithError("open() in load_topics() failed\n");
	
	if(stat(TOPICSDB, &st) == 0)	/* Get the total length of the file */
		size=st.st_size;

	/* Read the file of the topics saved and load them in an array of struct for the topics */
	if(size>1)
		while((fscanf(fd, "%s %s %s %[^\n]", tmp, username, name, content)) > 0)
		{
			id_counter[TOPICCOUNTER] = strtol(tmp, NULL, 0);
			topic[id_counter[TOPICCOUNTER]].topicid=id_counter[TOPICCOUNTER];
			strcpy(topic[id_counter[TOPICCOUNTER]].creator, username);
			strcpy(topic[id_counter[TOPICCOUNTER]].name, name);
			strcpy(topic[id_counter[TOPICCOUNTER]].content, content);	
		}
	/*else
	{
		id_counter[TOPICCOUNTER]=0;
		fclose(fd);
		return 0;
	}
	*/
	id_counter[TOPICCOUNTER]+=1;
	
	fclose(fd);

	return 0;
}

int write_topics()
{
	FILE *fd;
	
	if((fd=fopen(TOPICSDB, "w")) < 0)
		DieWithError("open() in write_topics() failed\n");
	
	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
	{
		if(topic[j].topicid > 0)
			fprintf(fd, "%d %s %s %s\n", topic[j].topicid, topic[j].creator, topic[j].name, topic[j].content); 
	}

	fclose(fd);

	return 0;
}
/*
	Function used to create a topic, asking the client for the fields needed to complete the elements in the struct.
		Uses the function pong() defined in utils.c to send and receive the proper fields.
*/
int create_topics(int client_socket, int current_id)
{
	//FILE *fd;
	int namelen, contentlen;

	strcpy(topic[id_counter[TOPICCOUNTER]].name, ping(client_socket, "### TOPIC CREATION MENU ###\nInsert the topic name: ", AUTHLEN));
	strcpy(topic[id_counter[TOPICCOUNTER]].content, ping(client_socket, "Insert the content of the topic: ", CONTENTLEN));
	strcpy(topic[id_counter[TOPICCOUNTER]].creator, user[current_id].username);
	topic[id_counter[TOPICCOUNTER]].topicid=id_counter[TOPICCOUNTER];
	
	/*Remove the '\n' from the user's input fields */
	namelen=strlen(topic[id_counter[TOPICCOUNTER]].name);
	topic[id_counter[TOPICCOUNTER]].name[namelen-1]=0;
	contentlen=strlen(topic[id_counter[TOPICCOUNTER]].content);
	topic[id_counter[TOPICCOUNTER]].content[contentlen-1]=0;
	//if((fd=fopen(TOPICSDB, "a+")) < 0)
	//	DieWithError("open() failed\n");
	id_counter[TOPICCOUNTER]+=1;
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
int list_topics(int client_socket)
{
	//FILE *fd;
	//int nbytes=0, size;
	char tmp[]="\n";
	char ret_string[1000];
	struct stat st;
	int namelen, contentlen;

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
	{
		if(topic[j].topicid > 0)
		{
			memset(ret_string, 0, sizeof(ret_string));		// Aggiungi l'ID the topic
			strcpy(ret_string, topic[j].creator);
			strcat(ret_string, tmp);
			strcat(ret_string, topic[j].name);
			strcat(ret_string, tmp);
			strcat(ret_string, topic[j].content);
			strcat(ret_string, tmp);
			strcat(ret_string, tmp);
			send(client_socket, ret_string, strlen(ret_string), 0);
		}
	}
	
	ping(client_socket, "Press ENTER to continue", ANSSIZE);
	//if((fd=fopen(TOPICSDB, "r")) < 0)								/* Open the file */
	//	DieWithError("open() failed\n");

	//if(stat(TOPICSDB, &st) == 0)									/* Get the total length of the file */
	//	size=st.st_size;
	/*
	if(size > 1)
	{
		ret_string=(char *)malloc(size+strlen(tmp));
	
		while((nbytes=fread(ret_string, sizeof(char), size, fd)) > 0)	/* Read from the file and store the content in a buffer */
	//		continue;

		//ret_string=realloc(ret_string, size+sizeof(tmp));				/* Update the size of the return string */
	//	strcat(ret_string, tmp);										/* Append a new string at the end of the buffer */
	//	pong(client_socket, ret_string, ANSSIZE);						/* Send the message to the client */
	//}
	//else
	//	pong(client_socket, "Topic file is empty!", ANSSIZE);
	
	//fclose(fd);

	return 0;
}

/*
	The client choose the ID of a topic and then the server will delete it only if the client
		is also the owner of the topic.
*/
int delete_topic(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id;

	strcpy(id_char, ping(client_socket, "Choose the topic ID you want to DELETE: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);

	if(strcmp(user[current_id].username, topic[id].creator) == 0)
	{
		memset(topic[id].creator, 0, sizeof(topic[id].creator));
		memset(topic[id].name, 0, sizeof(topic[id].name));
		memset(topic[id].content, 0, sizeof(topic[id].content));
		topic[id].topicid = -1;
	}
	//Rispondi al client?
	
	return 0;
}

/* 
	Append a new message to a topic.
		Ask the client which topic he wants to reply to(asking the ID) and
			create a new instance of an array of struct of messages with inside the ID of the topic.
*/
int reply(int client_socket, int current_id)
{
	/*
		typedef struct messages {
		int topic_id;	// cambia nome
		char msg_content[MSGLEN];
		// STATUS
		char msg_creator[AUTHLEN];
		} msg;
		msg *message;
	*/
	char id_char[ANSSIZE];
	int id;
	char failed[] = "Wrong topic ID!";

	strcpy(id_char, ping(client_socket, "Choose the topic ID on which you want to REPLY: ", ANSSIZE));
	id=strtol(id_char, NULL, 0);
	/* da fixare */
	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)
		if(id != topic[j].topicid && j==id_counter[TOPICCOUNTER]-1)
		{
			send(client_socket, failed, strlen(failed), 0);
			return 0;
		}	

	message[id_counter[MSGCOUNTER]].topic_id = id;
	strcpy(message[id_counter[MSGCOUNTER]].msg_creator, user[current_id].username);
	strcpy(message[id_counter[MSGCOUNTER]].msg_content, ping(client_socket, "Insert the content of the message: ", MSGLEN));

	return 0;
}