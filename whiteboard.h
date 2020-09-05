#define _GNU_SOURCE
#include <stdio.h>  
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

/* -- Stuff for files in utils.c -- */
#define BUFFSIZE 256
char buff[BUFFSIZE];

/* -- General stuff -- */
#define ANSSIZE 3							/* Used when expecting a number or the \n as answer to a ping() */
#define MENU "\n****** WHITEBOARD MENU ******\n\n \
1) Create a new topic\n \
2) List topics\n \
3) Delete a topic\n \
4) Reply to a thread\n \
5) Create a thread\n \
6) Display a topic content\n \
7) Subscribe to a topic\n \
8) Show unread messages\n \
9) Unsubscribe a topic\n \
10) Quit\n \
choose an operation: "
/* Both used only in server.c */
int server_socket, 							/* Server Socket FD */
client_socket;								/* Client Socket FD */

/* -- Stuff for the Socket management - LOCATION: whiteboard_sock.c -- */
#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL IPPROTO_TCP
#define MAXPENDING 5

/* -- Stuff for shared memory management - LOCATION: whiteboard_shm.c -- */
#define SHMPERM 0600						/* Perms for the shared memory segments */
#define SHMKEY_A 0x11111					/* Key for the authentication */
#define SHMKEY_T 0x22222					/* Key for the topics */
#define SHMKEY_C 0x33333					/* Key for counter */
#define SHMKEY_TH 0x44444					/* Key for the threads */
#define SHMKEY_M 0x55555					/* Key for the messages */
#define SHMSIZE 1000						/* Size of the structs in the shared memory */
#define COUNTSIZE 16						/* Size for the array for the 4 counters(4 int) stored in the shared memory */
int shmid_auth;	
int shmid_topics;
int shmid_counter;
int shmid_thread;
int shmid_msg;

/* -- Fields of the array for the IDs counter -- */
#define AUTHCOUNTER 0						/* id_counter[AUTHCOUNTER] */
#define TOPICCOUNTER 1						/* id_counter[TOPICCOUNTER] */
#define THREADCOUNTER 2						/* id_counter[TOPICCOUNTER] */
#define MSGCOUNTER 3						/* id_counter[MSGCOUNTER] */
int *id_counter;	/* Counter defined in shared memory used to know the number of clients/topics/threads/messages */

/* -- Stuff for semaphores management - LOCATION: whiteboard_sem.c -- */
#define SEMPERM 0600						/* Perms for the semaphores */
#define SEMKEY 0x66666						/* Key for the semaphores */
#define SEMAUTH 0							/* Semaphores used for the authentication method */
#define SEMTOPICS 1							/* Semaphores used for all the topics related creation/removal */
#define NUMSEM 2							/* Number of semaphores */
int semid;

/* -- Stuff for authentication process - LOCATION: whiteboard_auth.c -- */
#define AUTHLEN 20							/* Max size for usernames and passwords */
#define MAXSUBS 5							/* Max topics I can subscribes */
#define MAXUNREAD 10						/* Max messages unreaded that can be stored */
#define CREDFILE "db/credentials.txt"
#define SUBFILE "db/subscriptions.txt"
#define UNREADMSG "db/unread_msg.txt"
typedef struct authentication {				/* Struct to contain things I need for users */
	char username[AUTHLEN];
	char password[AUTHLEN];
	int logged;								/* 1 = Logged - 0 = Not logged */
	int usrid;								/* ID of the user when connecting to the server based on id_counter[AUTHCOUNTER] */
	int pid;								/* PID of the process that is managing the client */
	int topics_sub[MAXSUBS];				/* Topics at which the client is subscribed */
	int unread_msg[MAXUNREAD];				/* Messages market as UNREAD for the client */
} auth_user;
auth_user *user;

/* -- Stuff used for topics - LOCATION: whiteboard_topics.txt -- */
#define TOPICSDB "db/topics.txt"
typedef struct topics {						/* Struct to contain things I need for topics */
	int topicid;							/* ID of the topic */
	char name[AUTHLEN];						/* Name of the topic */
	char creator[AUTHLEN]; 					/* Creator of the topic = user creating this topic */
} tpc;
tpc *topic;

/* -- Stuff for the threads - LOCATION: whiteboard_threads.c -- */
#define CONTENTLEN 100						/* Max size of the content for threads and also messages*/
#define THREADDB "db/threads.txt"
typedef struct threads { 					/* Struct to contain things I need for threads */
	int topicid;							/* ID of the topic at which this thread belongs */
	int threadid;							/* ID of the thread */
	char name[AUTHLEN];
	char content[CONTENTLEN];
	char creator[AUTHLEN];
} thrd;
thrd *thread;

/* -- Stuff for the messages - whiteboard_messages.c -- */
#define MSGDB "db/messages.txt"
typedef struct messages {					/* Struct to contain things I need for messages */
	int msgid;								/* ID of the message */
	int threadid;							/* ID of the thread at thich this message belongs */
	char content[CONTENTLEN];
	char creator[AUTHLEN];
} msg;
msg *message;

/*
	NOTE: topics, threads and messages are related and I can recognize them, by checking their IDs,
		where the messages are related to a thread thanks to the threadid variable stored in both structures,
			and threads are related to topics for same reason(with topicid variable).
	message.threadid -> (thread.threadid -> thread.threadid) -> topic.topicid
	In this way I can recognize the relation between each element.
*/

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
int check_number();

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
