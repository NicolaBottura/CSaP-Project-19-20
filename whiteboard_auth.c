#include "whiteboard.h"

int authentication(int client_socket)
{
	FILE *fd;
	char name[AUTHLEN], passwd[AUTHLEN];
	
	user.logged=0; /* Set the variabled logged as 0 */

	strcpy(name, pong(client_socket, "Username: "));	/* Send to the client the string to ask a username */
	strcpy(passwd, pong(client_socket, "Password: "));	/* Send to the client the string to ask a password */

	if((fd=fopen(CREDFILE, "r")) < 0)	/* Open the file in read mode with the credentials of real users */
		DieWithError("open failed\n");
	
	/* Check each line of the file, and if a pair username:password matches with the client input change the 
		value of logged and print a message.
		If no match just exit from the while and print a related message */
	while((fscanf(fd, "%s %s", user.username, user.password)) > 0)
	{
		if((strcmp(name, user.username) == 0) && ((strcmp(passwd, user.password) == 0)))
		{
			user.logged=1;
			break;
		}
		else
			continue;
	}

	/* Just send to the client if the loging was good or not */
	if(user.logged == 1)
		pong(client_socket, "OK!");
	else if (user.logged == 0)
		pong(client_socket, "Login failed!");

	fclose(fd);		/* Close the file */

	return 0;
}