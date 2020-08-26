#include "whiteboard.h"

int load_users()
{
	FILE *fd;
	char name[AUTHLEN], passwd[AUTHLEN];

	if((fd=fopen(CREDFILE, "r")) < 0)
		DieWithError("open() in load_users()) failed\n");

	while((fscanf(fd, "%s %s", user[id_counter[AUTHCOUNTER]].username, user[id_counter[AUTHCOUNTER]].password)) > 0)
	{
		for(int j=0; j<MAXSUBS; j++)	/* load the topics subscribed by this user */
			fscanf(fd, "%d", &user[id_counter[AUTHCOUNTER]].topics_sub[j]);

		user[id_counter[AUTHCOUNTER]].logged=0;
		
		id_counter[AUTHCOUNTER]+=1;
	}

	fclose(fd);

	return 0;
}

int write_users()
{
	FILE *fd;
	
	if((fd=fopen(CREDFILE, "w")) < 0)		/* Open the file and overwrite the content with the new one */
		DieWithError("open() in write_users() failed\n");

	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)
	{
		fprintf(fd, "\n%s %s", user[j].username, user[j].password);
		for(int i=0; i<MAXSUBS; i++)	/* write the topics subscribed by this user */
			fprintf(fd, " %d", user[j].topics_sub[i]);
	}

	fclose(fd);

	return 0;
}
// INT LOAD_MSG_UNREAD

/* 
	Function used to authenticate the users once they connect to the server's listening socket.
		Ask for username and password using the function pong() defined in utils.c which just
			send a message and receive the client's answer.		
*/
int authentication(int client_socket)
{
	char name[AUTHLEN], passwd[AUTHLEN];	
	int namelen, passlen;

	user[id_counter[AUTHCOUNTER]].logged=0;

	/* Send to the client the string to ask a username and copy the answer inside the variables */
	strcpy(name, ping(client_socket, "*** Welcome to Whiteboard ***\nUsername: ", AUTHLEN));
	strcpy(passwd, ping(client_socket, "Password: ", AUTHLEN));	/* Send to the client the string to ask a password */

	/* Remove the '\n' from the user's input credentials */
	namelen=strlen(name);
	name[namelen-1]=0;
	passlen=strlen(passwd);
	passwd[passlen-1]=0;

	if(check_if_logged(name) < 0)
		DieWithError("This user is already logged\n");

	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)
		if((strcmp(name, user[j].username) == 0) && ((strcmp(passwd, user[j].password) == 0)))
		{
			user[j].logged=1;						/* If login was successful.. */
			user[j].usrid=j;						/* set the client ID equal to the value of the counter */
			user[j].pid=getpid();					/* set the PID equal to the PID of process who is managing this client */
			ping(client_socket, "Login Successful!\nPress ENTER to continue", ANSSIZE);		/* and send it to the client waiting for a 1 digit char */

			return 0;
		}
	
	ping(client_socket, "Login Failed!", 0);

	return -1;
}

/*
	Check if the user with the name passed is already logged, if yes, send the Login Failed string and exit
*/
int check_if_logged(char name[])
{
	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)
		if(strcmp(user[j].username, name) == 0 && user[j].logged == 1)
		{
			ping(client_socket, "Login Failed!", 0);
			v(SEMAUTH);		/* sem+1 for AUTH */
			return -1;
		}

	return 0;
}