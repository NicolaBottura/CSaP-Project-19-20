#include "whiteboard.h"

/*
	Load all the topics stored in the backup file in an array of structs for the topics.
*/
int load_threads()
{
	FILE *fd;
	struct stat st;
	char tmp1[ANSSIZE], tmp2[ANSSIZE], username[AUTHLEN], name[AUTHLEN], content[CONTENTLEN];
	int size;

	if((fd=fopen(THREADDB, "r")) < 0)
		DieWithError("open() in load_threads() failed\n");
	
	if(stat(THREADDB, &st) == 0)	/* Get the total length of the file */
		size=st.st_size;

	/* Read the file of the topics saved and load them in an array of struct for the topics */
	if(size>1)
		while((fscanf(fd, "%s %s %s %s %[^\n]", tmp1, tmp2, username, name, content)) > 0)
		{
			id_counter[THREADCOUNTER] = strtol(tmp1, NULL, 0);
			thread[id_counter[THREADCOUNTER]].threadid=id_counter[THREADCOUNTER];
			thread[id_counter[THREADCOUNTER]].topicid = strtol(tmp2, NULL, 0);
			strcpy(thread[id_counter[THREADCOUNTER]].creator, username);
			strcpy(thread[id_counter[THREADCOUNTER]].name, name);
			strcpy(thread[id_counter[THREADCOUNTER]].content, content);
		}

	id_counter[THREADCOUNTER]+=1;

	fclose(fd);

	return 0;
}

int write_threads()
{
	FILE *fd;
	
	if((fd=fopen(THREADDB, "w")) < 0)		/* Open the file and overwrite the content with the new one */
		DieWithError("open() in write_threads() failed\n");
	
	for(int j=0; j<id_counter[THREADCOUNTER]; j++)
	{
		if(thread[j].threadid > 0)	/* Check that the topics exists - if not, the id is = -1 */
			fprintf(fd, "%d %d %s %s %s\n", thread[j].threadid, thread[j].topicid, thread[j].creator, thread[j].name, thread[j].content); 
	}

	fclose(fd);

	return 0;
}
/*
	Function used to append a new message(thread content) to a new thread under a specific topic.
		Ask for the topic ID, write the name and the content of this thread.
*/
int append(int client_socket, int current_id)
{
	char id_char[ANSSIZE];
	int id, namelen, contentlen;

	strcpy(id_char, ping(client_socket, "Choose the ID of the topic in which you want to append the thread: ", ANSSIZE));
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
			thread[id_counter[THREADCOUNTER]].threadid=id_counter[THREADCOUNTER]; 
			thread[id_counter[THREADCOUNTER]].topicid=id;
			strcpy(thread[id_counter[THREADCOUNTER]].creator, user[current_id].username);
			strcpy(thread[id_counter[THREADCOUNTER]].name, ping(client_socket, "Insert the name of this thread: ", AUTHLEN));
			strcpy(thread[id_counter[THREADCOUNTER]].content, ping(client_socket, "Insert the content of this thread: ", CONTENTLEN));
	
			/*Remove the '\n' from the user's input fields */
			namelen=strlen(thread[id_counter[THREADCOUNTER]].name);
			thread[id_counter[THREADCOUNTER]].name[namelen-1]=0;
			contentlen=strlen(thread[id_counter[THREADCOUNTER]].content);
			thread[id_counter[THREADCOUNTER]].content[contentlen-1]=0;

			id_counter[THREADCOUNTER]+=1;

			return 0;
		}
	}
}