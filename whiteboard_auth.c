#include "whiteboard.h"

void authentication(int client_socket)
{
	FILE *fd;
	char name[AUTHLEN], passwd[AUTHLEN];
	char ret_string[BUFFSIZE];

	user->logged=0; /* Set the variabled logged as 0 */

	/* Send to the client the string to ask a username */
	strcpy(name, pong(client_socket, "*** Welcome to Whiteboard ***\nUsername: "));
	strcpy(passwd, pong(client_socket, "Password: "));	/* Send to the client the string to ask a password */

	if((fd=fopen(CREDFILE, "r")) < 0)	/* Open the file in read mode with the credentials of real users */
		DieWithError("open failed\n");
	
	/* Check each line of the file, and if a pair username:password matches with the client input change the 
		value of logged and print a message.
		If no match just exit from the while and print a related message */
	while((fscanf(fd, "%s %s", user->username, user->password)) > 0)
	{
		if((strcmp(name, user->username) == 0) && ((strcmp(passwd, user->password) == 0)))
		{
			user->logged=1;
			break;
		}
		else
			continue;
	}

	/* Just send to the client if the loging was good or not */
	if(user->logged == 1)
	{
		strcpy(ret_string, "Login Successful!\n\n");
		strcat(ret_string, MENU);
		pong(client_socket, ret_string);
	}
	else if (user->logged == 0)
	{
		strcpy(ret_string, "Login Failed!");
		pong(client_socket, ret_string);
	}

	fclose(fd);		/* Close the file */

	return ;
}