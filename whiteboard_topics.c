#include "whiteboard.h"

/*
	Function used to create a topic, asking the client for the fields needed to complete the elements in the struct.
		Uses the function pong() defined in utils.c to send and receive the proper fields.
*/
int create_topics(int client_socket, int current_id)
{
	FILE *fd;
	time_t t=time(NULL);
	struct tm *tm = localtime(&t);

	strcpy(topic->name, pong(client_socket, "### TOPIC CREATION MENU ###\nInsert the topic name: ", NAMELEN));
	strcpy(topic->content, pong(client_socket, "Insert the content of the topic: ", CONTENTLEN));

	if((fd=fopen(TOPICSDB, "a+")) < 0)
		DieWithError("open() failed\n");

	/* Write in the file the fields of the new topic */
	fprintf(fd, "Name: %sTime: %sCreator: %s\nContent: %s\n", topic->name, asctime(tm), user[current_id].username, topic->content);

	fclose(fd);

	return 0;
}

int list_topics(int client_socket)
{
	FILE *fd;
	int nbytes=0;
	char tmp[BUFFSIZE], ret_string[BUFFSIZE];
;

	if((fd=fopen(TOPICSDB, "r")) < 0)
		DieWithError("open() failed\n");

	// FARE LA READ DAL FILE

	strcpy(ret_string, tmp);
	strcat(ret_string, "\nPress ENTER to return the menu\n");

//	pong(client_socket, ret_string);

	fclose(fd);

	return 0;
}