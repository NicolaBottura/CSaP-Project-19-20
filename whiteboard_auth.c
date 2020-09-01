#include "whiteboard.h"

/*
	Load all the credentials in the array of struct of the users.
	(credentials.txt) File format: username password
*/
int load_users()
{
	FILE *fd;

	if((fd=fopen(CREDFILE, "r")) < 0)						/* Open the file in READ mode */
		DieWithError("open() in load_users()) failed\n");

	while((fscanf(fd, "%s %s", user[id_counter[AUTHCOUNTER]].username, user[id_counter[AUTHCOUNTER]].password)) > 0)
	{
		user[id_counter[AUTHCOUNTER]].logged=0;				/* Set all the user as not logged in */
		id_counter[AUTHCOUNTER]+=1;							/* Increment the counter for the users by 1 */
	}

	fclose(fd);

	return 0;
}			// TOGLI LA PASSWORD IN CHIARO QUANDO L'UTENTE SI LOGGA

/*
	Load all the unread-message IDs and the subscribed topics for each user.
	(subscriptions.txt) Format for the subscriptions:	1 2 0 6... if(ID>0); existing(topic)=TRUE;
	(unread_msg.txt) Formato for the unread: 			0 0 4 0 5 6 ... same as above
*/
void load_utils()
{
	FILE *fd1, *fd2;
	struct stat st;
	int size;

	if((fd1=fopen(SUBFILE, "r")) < 0)						/* Open the file in READ mode */
		DieWithError("open() in write_users() failed\n");
	
	if((fd2=fopen(UNREADMSG, "r")) < 0)						/* Open the file in READ mode */
		DieWithError("open() in write_users() failed\n");

	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)			/* For each user */
		for(int i=0; i<MAXSUBS; i++)						/* for each possible subscription */
			fscanf(fd1, "%d", &user[j].topics_sub[i]);		/* load the topic subscribed by the user */
	
	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)			/* Same as above but for unread messages */
		for(int i=0; i<MAXUNREAD; i++)
			fscanf(fd2, "%d", &user[j].unread_msg[i]);

	fclose(fd1);
	fclose(fd2);

	return;
}

/*
	Write in the related file the IDs for the topics subscribed and messages unread for each user.
	NOTE: I prefer to not modify the credentials file overwriting it with the credentials for security reasons,
		so it will be always the same at each run of the program and the write will happen only when the server crashes
			so, not everytime I add/remove something from the structs because opening and closing the files multiple times will
				produce too much overhead.
*/
void write_utils()
{
	FILE *fd1, *fd2;

	if((fd1=fopen(SUBFILE, "w")) < 0)						/* Open the file in WRITE mode */
		DieWithError("open() in write_users() failed\n");
	
	if((fd2=fopen(UNREADMSG, "w")) < 0)						/* Open the file in WRITE mode */
		DieWithError("open() in write_users() failed\n");

	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)			/* For each user */
	{
		for(int i=0; i<MAXSUBS; i++)						/* for each possible subscription */
			fprintf(fd1, "%d ", user[j].topics_sub[i]);		/* write in the file the id of the topics inside the array */
		fprintf(fd1, "\n");
	}

	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)			/* Same as above for the unread messages */
	{
		for(int i=0; i<MAXUNREAD; i++)
			fprintf(fd2, "%d ", user[j].unread_msg[i]);
		fprintf(fd2, "\n");
	}

	fclose(fd1);
	fclose(fd2);

	return;
}

/* 
	Function used to authenticate the users once they connect to the server listening socket and called in server.c.
	Ask for username and password using the function ping() defined in utils.c which just
		send a message and return the client's answer.
*/
int authentication(int client_socket)
{
	char name[AUTHLEN], passwd[AUTHLEN];
	int namelen, passlen;

	/* Send to the client the string to ask a username and copy the answer inside the variables(same for password) */
	strcpy(name, ping(client_socket, "*** Welcome to Whiteboard ***\nUsername: ", AUTHLEN));
	strcpy(passwd, ping(client_socket, "Password: ", AUTHLEN));

	/* Remove the '\n' from the user's input credentials */
	namelen=strlen(name);
	name[namelen-1]=0;
	passlen=strlen(passwd);
	passwd[passlen-1]=0;

	if(check_if_logged(name) < 0)							/* Check if the user with the name provided is alreay logged in */
		DieWithError("This user is already logged\n");		/* If it returns a value < 0 means that the user is already logged, so, call DieWithError to kill the client */

	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)			/* For each user in the struct, check if the username and password provided match with an existing pair */
		if((strcmp(name, user[j].username) == 0) && (strcmp(passwd, user[j].password) == 0))
		{
			user[j].logged=1;								/* If login was successful set the user as logged */
			user[j].usrid=j;								/* set the client ID equal to the value of the counter */
			user[j].pid=getpid();							/* set the pid equal to the PID of process who is managing this client */

			return 0;										/* The just return */
		}


	ping(client_socket, "Login Failed!", 0);				/* Otherwise send the response to the client */

	return -1;
}

/*
	Check if the user with the name passed is already logged, by checking its user[j].logged variable.
	If yes, send the Login Failed string and exit.

	The response from the server, if the user is already logged, is the same as if the credentials were wrong to
		try to mitigate possible brute-force attacks to the login system by harvesting the response from the server.
*/
int check_if_logged(char name[])
{
	for(int j=0; j<id_counter[AUTHCOUNTER]; j++)
		if(strcmp(user[j].username, name) == 0 && user[j].logged == 1)
		{
			ping(client_socket, "Login Failed!", 0);
			return -1;
		}

	return 0;
}