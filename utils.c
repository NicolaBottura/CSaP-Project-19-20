#include "whiteboard.h"

/* 
	Print an error message and exits.
*/
void DieWithError(char *message)
{
	perror(message);
	exit(1);
}

/*
	When the interrupt signal occurs, write down in the files all the things stored in the struct,
		remove the shared memory segment and the semaphores, close the sockets and exit.
*/
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
	exit(1);
}

/* Function used to send a message back to the client through the socket 
	and get back the response by it.
	Server: ping to client
	Client: pong back
*/
char *ping(int client_socket, char *message, int reponse_len)
{
	int msg_len, bytesreceived, current_id;
	char error[] = "Exiting the program\nBYE!";

	msg_len = strlen(message);
	memset(buff, 0, sizeof(buff));

	if((send(client_socket, message, msg_len, 0)) != msg_len)
		DieWithError("send() failed\n");

	if((bytesreceived=recv(client_socket, buff, sizeof(buff), 0)) < 0)
		DieWithError("recv() failed\n");
	
	buff[bytesreceived]='\0';

	if(strlen(buff) > reponse_len || bytesreceived <= 0)				/* If the asnwer provided is longer than the expected one, or I receive nothing(used for ctrl+c), the client exits */ 
	{
		current_id=getcurrentid();
		user[current_id].logged=0;
		v(SEMAUTH);														/* Call the v-operation for both sempahores */
		v(SEMTOPICS);													/* NOTE: this may cause not blocking operations for each time the v-operation was called if multiple clients crash togheter */
		send(client_socket, error, sizeof(error), 0);
		DieWithError("[PING]: Max length exceeded or connection closed on client side\n");
	}

	return buff;
}

/* 
	Function that returns the ID of the client on which I want to do an operation.
	This is possible by checking the PID of the process that invokes this function 
		and comparing it to the PID stored in the struct of users.
*/
int getcurrentid()
{
	for(int j=0; j<id_counter[AUTHCOUNTER]; j++){	
		if(user[j].pid == getpid())
			return user[j].usrid;
	}

	return -1;	
}