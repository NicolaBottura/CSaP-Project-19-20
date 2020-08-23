#include "whiteboard.h"

void DieWithError(char *message)
{
	perror(message);
	exit(1);
}

void sigint(int signal)
{
	write_topics();
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
	int msg_len, bytesreceived;

	msg_len = strlen(message);
	memset(buff, 0, sizeof(buff));
	//printf("[3] I'm pong() with message = %s\n", message); // MESSAGGIO DI CONTROLLO - ELIMINARE

	if((send(client_socket, message, msg_len, 0)) != msg_len)
		DieWithError("send() failed\n");

	if((bytesreceived=recv(client_socket, buff, sizeof(buff), 0)) < 0)
		DieWithError("recv() failed\n");
	
	buff[bytesreceived]='\0';

	if(strlen(buff) > reponse_len)		/* +1 because there is the \n at the end */
	{
		v(SEMAUTH);	// PROVVISORIO: se muoio qui devo sbloccare il semaforo
		DieWithError("Length of the message received higher than that requested size\n");
	}

	//printf("Received: %s\n", buff);		// MESSAGGIO DI CONTROLLO - ELIMINARE
	
	return buff;
}

char *send_only(int client_socket, char *message1, char *message2)
{
	strcat(message1, message2);

	send(client_socket, message1, strlen(message1), 0);
}