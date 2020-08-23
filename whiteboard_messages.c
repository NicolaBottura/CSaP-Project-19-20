#include "whiteboard.h"

int load_messages()
{
	FILE *fd;
	struct stat st;
	int size;
	char tmp[ANSSIZE], content[MSGLEN], creator[AUTHLEN];

	if((fd=fopen(MSGDB, "r")) < 0)
		DieWithError("open() in load_messages() failed\n");
	
	if(stat(MSGDB, &st) == 0)	/* Get the total length of the file */
		size=st.st_size;

	/* Read the file of the topics saved and load them in an array of struct for the topics */
	if(size>1)
		while((fscanf(fd, "%s %s %[^\n]", tmp, creator, content)) > 0)
		{
			message[id_counter[MSGCOUNTER]].topic_id=strtol(tmp, NULL, 0);	/* Get from the the file the ID of the topic */
			strcpy(message[id_counter[MSGCOUNTER]].msg_creator, creator);
			strcpy(message[id_counter[MSGCOUNTER]].msg_content, content);
			id_counter[MSGCOUNTER]+=1;				/* +1 the number of messages */
		}

	fclose(fd);

	return 0;
}

int write_messages()
{
	FILE *fd;
	
	if((fd=fopen(MSGDB, "w")) < 0)		/* Open the file and overwrite the content with the new one */
		DieWithError("open() in write_messages() failed\n");
	
	for(int j=0; j<id_counter[MSGCOUNTER]; j++)
			fprintf(fd, "%d %s %s\n", message[j].topic_id, message[j].msg_creator, message[j].msg_content); 

	fclose(fd);

	return 0;
}

/* 
	Append a new message to a topic.
		Ask the client which topic he wants to reply to(asking the ID) and
			create a new instance of an array of struct of messages with inside the ID of the topic.
*/
int reply(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id, namelen, contentlen;

	// IPMPORTANTE -> IL TOPIC NUMERO 0 ESISTE ANCHE SE NON C'E' NIENTE DENTRO E POSSO QUINDI FARE LA REPLY, FIXARE STA COSA ASAP 

	strcpy(id_char, ping(client_socket, "Choose the topic ID on which you want to REPLY: ", ANSSIZE));		/* Ask the ID of the topic */
	id=strtol(id_char, NULL, 0);																			/* Convert it in an int type */

	for(int j=0; j<id_counter[TOPICCOUNTER]; j++)	/* Check that the ID of the topic exists */
	{
		if(id >= id_counter[TOPICCOUNTER] || (topic[j].topicid != id && j==id_counter[TOPICCOUNTER]-1))
		{
			ping(client_socket, "This topic does not exist!\nPress ENTER to continue!", ANSSIZE);
			return 0;
		}
		else if(topic[j].topicid == id)
		{
			message[id_counter[MSGCOUNTER]].topic_id = id;
			strcpy(message[id_counter[MSGCOUNTER]].msg_creator, user[current_id].username);
			strcpy(message[id_counter[MSGCOUNTER]].msg_content, ping(client_socket, "Insert the content of the message: ", MSGLEN));
	
			/*Remove the '\n' from the user's input fields */
			contentlen=strlen(message[id_counter[MSGCOUNTER]].msg_content);
			message[id_counter[MSGCOUNTER]].msg_content[contentlen-1]=0;

			id_counter[MSGCOUNTER]+=1;

			return 0;
		}
	}
}