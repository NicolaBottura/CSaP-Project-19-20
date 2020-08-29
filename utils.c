#include "whiteboard.h"

void DieWithError(char *message)
{
	perror(message);
	exit(1);
}

void sigint(int signal)
{
	write_utils();
	write_topics();
	write_threads();
	write_messages();
	remove_shm();
	remove_sem();
	close(server_socket);
	close(client_socket);
	
	printf("Signal %d occurred, exiting.\n", signal);
	exit(0);
}

/* Function used to send a message back to the client through the socket 
	and get back the response by it.
	Server: ping
	Client: pong back
*/
char *ping(int client_socket, char *message, int reponse_len)
{
	int msg_len, bytesreceived, current_id;
	char error[] = "Exiting the program\nBYE!";

	msg_len = strlen(message);
	memset(buff, 0, sizeof(buff));
	//printf("[3] I'm pong() with message = %s\n", message); // MESSAGGIO DI CONTROLLO - ELIMINARE

	if((send(client_socket, message, msg_len, 0)) != msg_len)
		DieWithError("send() failed\n");

	if((bytesreceived=recv(client_socket, buff, sizeof(buff), 0)) < 0)
		DieWithError("recv() failed\n");
	
	buff[bytesreceived]='\0';

	if(strlen(buff) > reponse_len || bytesreceived <= 0)		/* +1 because there is the \n at the end */
	{
		current_id=getcurrentid();
		user[current_id].logged=0;
		v(SEMAUTH);
		v(SEMTOPICS);
		send(client_socket, error, sizeof(error), 0);
		DieWithError("Max length exceeded or connection close on client side\n");
	}

	//printf("Received: %s\n", buff);		// MESSAGGIO DI CONTROLLO - ELIMINARE
	
	return buff;
}

/* 
	Function that returns the ID of the client on which I want to do an operation.
		This is possible by checking the PID of the process that invokes this function.
*/
int getcurrentid()
{
	for(int j=0; j<id_counter[AUTHCOUNTER]; j++){	
		if(user[j].pid == getpid())
			return user[j].usrid;
	}

	return -1;	
}