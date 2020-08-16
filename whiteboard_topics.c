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

/*
	Function that lists the content of the file in which the topics are stored.
*/
int list_topics(int client_socket)
{
	FILE *fd;
	int nbytes=0, size;
	char tmp[]="\nPress ENTER to continue", 
	*ret_string;
	struct stat st;

	if((fd=fopen(TOPICSDB, "r")) < 0)								/* Open the file */
		DieWithError("open() failed\n");

	if(stat(TOPICSDB, &st) == 0)									/* Get the total length of the file */
		size=st.st_size;

	ret_string=(char *)malloc(size);
	
	while((nbytes=fread(ret_string, sizeof(char), size, fd)) > 0)	/* Read from the file and store the content in a buffer */
		continue;

	ret_string=realloc(ret_string, size+sizeof(tmp));				/* Update the size of the return string */
	strcat(ret_string, tmp);										/* Append a new string at the end of the buffer */

	pong(client_socket, ret_string, ANSSIZE);						/* Send the message to the client */

	fclose(fd);

	return 0;
}