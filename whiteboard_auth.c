#include "whiteboard.h"

/* 
	Function used to authenticate the users once they connect to the server's listening socket.
		Ask for username and password using the function pong() defined in utils.c which just
			send a message and receive the client's answer.		
*/
int authentication(int client_socket)
{
	FILE *fd;
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

	if((fd=fopen(CREDFILE, "r")) < 0)	/* Open the file in read mode with the credentials of real users */
		DieWithError("open failed\n");

	/* Check each line of the file, and if a pair username:password matches with the client input change the 
		value of logged and print a message.
		If no match just exit from the while and print a related message */
	while((fscanf(fd, "%s %s", user[*id_counter].username, user[*id_counter].password)) > 0)
	{
		if((strcmp(name, user[id_counter[AUTHCOUNTER]].username) == 0) && ((strcmp(passwd, user[id_counter[AUTHCOUNTER]].password) == 0)))
		{
			user[id_counter[AUTHCOUNTER]].logged=1;
			break;
		}
		else
			continue;
	}

	/* Just send to the client if the loging was good or not */
	if(user[id_counter[AUTHCOUNTER]].logged == 1)
	{													/* If login was successful.. */
		user[id_counter[AUTHCOUNTER]].usrid=*id_counter;			/* set the client ID equal to the value of the counter */
		user[id_counter[AUTHCOUNTER]].pid=getpid();					/* set the PID equal to the PID of process who is managing this client */
		id_counter[AUTHCOUNTER]+=1;									/* increase the counter by 1 */
		ping(client_socket, "Login Successful!\nPress ENTER to continue", ANSSIZE);		/* and send it to the client waiting for a 1 digit char */
	}
	else if (user[id_counter[AUTHCOUNTER]].logged == 0) // CONTROLLA CHE SOVRASCRIVA LA MEMORIA AL POST ID_COUNTER SE FALLISCE
		ping(client_socket, "Login Failed!", 0);
	

	fclose(fd);		/* Close the file */

	return 0;
}