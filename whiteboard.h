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
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define BUFFSIZE 256

/* Stuff for files in utils.c */
char buff[BUFFSIZE];
#define MENU "****** WHITEBOARD MENU ******\n \
1) Create a topic\n \
2) List the topics\n \
0) Exit\n "

/* Stuff for the Socket creation/management - whiteboard_sock.c */
#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL IPPROTO_TCP
#define MAXPENDING 5

/* Stuff for shared memory creation/management - whiteboard_shm.c */
#define SHMPERM 0600
#define SHMKEY_A 0x12345
#define SHMKEY_T 0x54321
int shmid_auth;
int shmid_topics;
// RICORDA DI DEFINIRE LA SIZE DELLA SHM = SIZEOF(STRUCT)

/* Stuff for semaphores creation/management - whiteboard_sem.c */
#define SEMPERM 0600
#define SEMKEY 0x11111
#define AUTH_CS 1
#define NUMSEM 1	// METTERE A POSTO
int semid;

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
	int usrid;		/* ID of the user when connecting to the server */
} auth_user;
auth_user *user;

/* Stuff for the topics - whiteboard_topics.c */
#define NAMELEN 10
#define CONTENTLEN 100
#define TOPICSDB "topics.txt"
typedef struct topics_str {
	char name[NAMELEN];
	char content[CONTENTLEN];
	char creator[AUTHLEN]; 		/* Creator of the topic = user creating this topic */
} topics;
topics *topic;

/* Prototypes */
void DieWithError(char *message);
void sigint(int signal);
int init_shm(int perms);
int remove_shm();
int get_sem(int perms);
int init_sem();
int remove_sem();
int p(int semnum);
int v(int semnum);
char *pong(int client_socket, char *message);
int create_socket(unsigned short port);
int accept_connection(int server_socket);
int authentication(int client_socket);
int create_topics(int client_socket);
int list_topics(int client_socket);