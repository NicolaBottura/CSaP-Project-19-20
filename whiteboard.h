#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>		/* for signal() */
#include <sys/types.h>	/* for waitpid() */
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFSIZE 256

/* Stuff for files in utils.c */
char menu[] = "****** WHITEBOARD MENU ******\n \
1) Authentication\n \
2) List the topics\n \
0) Exit\n ";

/* Stuff for the Socket creation/management file whiteboard_sock. */
#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL IPPROTO_TCP
#define MAXPENDING 5

/* Stuff for shared memory creation/management file whiteboard_shm.c */
#define SHMPERM 0600
#define SHMKEY 0x121314
int shmid;

/* Both used only in server.c */
int server_socket, 		/* Server Socket FD */
client_socket;			/* Client Socket FD */

/* stuff for authentication process - whiteboard_auth.c */
#define AUTHLEN 20
#define CREDFILE "credentials.txt"
/* Struct to contain things I need for authentication */
typedef struct authentication {
	char username[AUTHLEN];
	char password[AUTHLEN];
	int logged;		/* 1 = Logged - 0 = Not logged */ 
} auth_user;
auth_user user;

/* Prototypes */
void DieWithError(char *message);
void sigint(int signal);
char pong(int client_socket, char *message)
int create_socket(unsigned short port);
int accept_connection(int server_socket);
int authentication(int client_socket)
