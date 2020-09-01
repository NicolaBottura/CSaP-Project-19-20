#define _GNU_SOURCE
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

/* Stuff for files in utils.c */
#define BUFFSIZE 256
char buff[BUFFSIZE];

#define ANSSIZE 3	/* Max size of certain client's answers will be a double digit numer + \n */
#define MENU "****** WHITEBOARD MENU ******\n \
1) Create a new topic\n \
2) List topics\n \
3) Delete a topic\n \
4) Reply to a thread\n \
5) Create a thread\n \
6) Display a topic content\n \
7) Subscribe to a topic\n \
8) Show unread messages\n \
9) Unsubscribe a topic\n \
10) Quit\n\n \
CHOOSE A NEW OPERATION: "

/* Stuff for the Socket creation/management - whiteboard_sock.c */
#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL IPPROTO_TCP
#define MAXPENDING 5

/* Stuff for shared memory creation/management - whiteboard_shm.c */
// RICORDA DI DEFINIRE LA SIZE DELLA SHM = SIZEOF(STRUCT)send
#define SHMPERM 0600		/* Perms for the shared memory segments */
#define SHMKEY_A 0x11111	/* Authentication */
#define SHMKEY_T 0x22222	/* Topics */
#define SHMKEY_C 0x33333	/* ID Counter for users and topics */
#define SHMKEY_TH 0x44444	/* Threads */
#define SHMKEY_M 0x55555	/* Messages */
#define SHMSIZE 1000		/* Size of the structs in the shared memory */
#define COUNTSIZE 16		/* Size for the array for the 4 counters stored in the shared memory */

int shmid_auth;
int shmid_topics;
int shmid_counter;
int shmid_thread;
int shmid_msg;

/* Fields of the array for the IDs counter */
#define AUTHCOUNTER 0		/* id_counter[AUTHCOUNTER] */
#define TOPICCOUNTER 1		/* id_counter[TOPICCOUNTER] */
#define THREADCOUNTER 2		/* id_counter[TOPICCOUNTER] */
#define MSGCOUNTER 3		/* id_counter[MSGCOUNTER] */

/* Stuff for semaphores creation/management - whiteboard_sem.c */
#define SEMPERM 0600
#define SEMKEY 0x66666
#define SEMAUTH 0
#define SEMTOPICS 1
#define NUMSEM 3	// METTERE A POSTO
int semid;

/* Both used only in server.c */
int server_socket, 		/* Server Socket FD */
client_socket;			/* Client Socket FD */

/* stuff for authentication process - whiteboard_auth.c */
#define AUTHLEN 20
#define MAXSUBS 5		/* Max topics I can subscribes */
#define MAXUNREAD 30
#define CREDFILE "db/credentials.txt"
#define SUBFILE "db/subscriptions.txt"
#define UNREADMSG "db/unread_msg.txt"
/* Struct to contain things I need for authentication */
typedef struct authentication {
	char username[AUTHLEN];
	char password[AUTHLEN];
	int logged;		/* 1 = Logged - 0 = Not logged */
	int usrid;		/* ID of the user when connecting to the server based on id_counter defined below */
	int pid;		/* PID of the process that is managing the client */
	int topics_sub[MAXSUBS];	/* add a unsub function */
	int unread_msg[MAXUNREAD];
} auth_user;
auth_user *user;
int *id_counter;	/* Counter defined in shared memory used to know the number of clients that have successfully logged in */

/* Stuff for the topics - whiteboard_topics.c */
#define TOPICSDB "db/topics.txt"
typedef struct topics {
	int topicid;
	char name[AUTHLEN];
	char creator[AUTHLEN]; 		/* Creator of the topic = user creating this topic */
} tpc;
tpc *topic;

/* Stuff for the threads - whiteboard_threads.c */
#define CONTENTLEN 100
#define THREADDB "db/threads.txt"
typedef struct threads
{
	int topicid;
	int threadid;
	char name[AUTHLEN];
	char content[CONTENTLEN];
	char creator[AUTHLEN];
} thrd;
thrd *thread;

/* Stuff for the messages - whiteboard_messages.c */
#define MSGDB "db/messages.txt"
typedef struct messages {
	int msgid;
	int threadid;
	char content[CONTENTLEN];
	char creator[AUTHLEN];
} msg;
msg *message;

/* -- utils.c -- */
void DieWithError(char *message);
void sigint(int signal);
char *ping(int client_socket, char *message, int response_len);
int getcurrentid();

/* -- whiteboard_shm.c -- */
int init_shm(int perms);
int remove_shm();

/* -- whiteboard_sem.c -- */
int get_sem(int perms);
int init_sem(int semvals[]);
int remove_sem();
int p(int semnum);
int v(int semnum);

/* -- whiteboard_sock.c -- */
int create_socket(unsigned short port);
int accept_connection(int server_socket);

/* -- whiteboard_auth.c -- */
void load_users();
void load_utils();
void write_utils();
int authentication(int client_socket);
int check_if_logged(char name[]);

/* -- whiteboard_topics.c -- */
void create_topics(int client_socket, int current_id);
void list_topics(int client_socket, int current_id);
void load_topics();
void write_topics();
void delete_topic(int client_socket, int current_id);
void subscribe(int client_socket, int current_id);
int unsubscribe(int client_socket, int current_id);

/* -- whiteboard_threads.c -- */
void load_threads();
void write_threads();
void append(int client_socket, int current_id);

/* -- whiteboard_messages.c -- */
void load_messages();
void write_messages();
void reply(int client_socket, int current_id);
void display_topic_content(int client_socket, int current_id);
void show_unread(int client_socket, int current_id);
