#include "whiteboard.h"

int authentication(int client_socket)
{
	FILE *fd;
	char name[AUTHLEN], passwd[AUTHLEN];
	
	user.logged=0;

	strcpy(name, pong(client_socket, "Username: "));
	strcpy(passwd, pong(client_socket, "Password: "));

	/*
	printf("Username: ");
	fgets(name, auth_length, stdin);
	printf("\nPassowrd: ");
	fgets(passwd, auth_length, stdin);
	printf("\n") */
	
	/* Remove the '\n' from the user input name and password */
	int namelen=strlen(name);
	name[namelen-1]=0;
	int passlen=strlen(passwd);
	passwd[passlen-1]=0;

	if((fd=fopen(CREDFILE, "r")) < 0)
		DieWithError("open failed\n");

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

	/* Just print if login failed or not */
	if(user.logged == 1)
		pong(client_socket, "OK!\n");
	else if (user.logged == 0)
		pong(client_socket, "Login failed!\n");

	fclose(fd);	

	return 0;
}