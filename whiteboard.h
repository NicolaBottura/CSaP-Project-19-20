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
#include <time.h>

#define BUFFSIZE 256

/* Stuff for files in utils.c */
char buff[BUFFSIZE];
#define ANSSIZE 2
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
#define SHMKEY_C 0x22222
int shmid_auth;
int shmid_topics;
int shmid_counter;
#define AUTHCOUNTER 0		/* id_counter[AUTHCOUNTER] */
#define TOPICCOUNTER 1		/* id_counter[TOPICCOUNTER] */

// RICORDA DI DEFINIRE LA SIZE DELLA SHM = SIZEOF(STRUCT)

/* Stuff for semaphores creation/management - whiteboard_sem.c */
#define SEMPERM 0600
#define SEMKEY 0x11111
#define SEMAUTH 0
#define SEMTOPICS 1
#define NUMSEM 2	// METTERE A POSTO
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
	int usrid;		/* ID of the user when connecting to the server based on id_counter defined below */
	int pid;		/* PID of the process that is managing the client */
} auth_user;
auth_user *user;
int *id_counter;	/* Counter defined in shared memory used to know the number of clients that have successfully logged in */

/* Stuff for the topics - whiteboard_topics.c */
#define NAMELEN 20
#define CONTENTLEN 100
#define TOPICSDB "topics.txt"
typedef struct topics_str {
	int topicid;
	char name[NAMELEN];
	char content[CONTENTLEN];
	char creator[AUTHLEN]; 		/* Creator of the topic = user creating this topic */
	char messages[1000];
	int messageid;
	char msg_creator[AUTHLEN];
} topics;
topics *topic;

/* Prototypes */
void DieWithError(char *message);
void sigint(int signal);
int init_shm(int perms);
int remove_shm();
int get_sem(int perms);
int init_sem(int semvals[]);
int remove_sem();
int p(int semnum);
int v(int semnum);
char *pong(int client_socket, char *message, int response_len);
int create_socket(unsigned short port);
int accept_connection(int server_socket);
int authentication(int client_socket);
int create_topics(int client_socket, int current_id);
int list_topics(int client_socket);
int load_topics();
char *send_only(int client_socket, char *message1, char *message2);
int write_topics();