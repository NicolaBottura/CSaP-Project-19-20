#include "whiteboard.h"

/* 
	Function that allocates and attaches a SYSV shared memory segmet for 
		id_count[4] - array used as counter for 4 different things
		user's struct
		topic's struct
		thread's struct
		message's struct
	Called in server.c
*/
int init_shm(int perms)
{
	if((shmid_counter=shmget(SHMKEY_C, COUNTSIZE, perms)) < 0)
		DieWithError("shmget()#1 failed\n");
	if((shmid_auth=shmget(SHMKEY_A, SHMSIZE, perms)) < 0)
		DieWithError("shmget()#2 failed\n");
	if((shmid_topics=shmget(SHMKEY_T, SHMSIZE, perms)) < 0)
		DieWithError("shmget()#3 failed\n");
	if((shmid_thread=shmget(SHMKEY_TH, SHMSIZE, perms)) < 0)
		DieWithError("shmget()#4 failed\n");
	if((shmid_msg=shmget(SHMKEY_M, SHMSIZE, perms)) < 0)
		DieWithError("shmget()#5 failed\n");

	if((id_counter=shmat(shmid_counter, NULL, 0)) < 0)
		DieWithError("shmat()#1 failed\n");
	if((user=(auth_user *)shmat(shmid_auth, NULL, 0)) < 0)
		DieWithError("shmat()#2 failed\n");
	if((topic=(tpc *)shmat(shmid_topics, NULL, 0)) < 0)
		DieWithError("shmat()#3 failed\n");
	if((thread=(thrd *) shmat(shmid_thread, NULL, 0)) < 0)
		DieWithError("shmat()#4 failed\n");
	if((message=(msg *)shmat(shmid_msg, NULL, 0)) < 0)
		DieWithError("shmat()#5 failed\n");

	return 0;
}

/*
	Function used to detach the shared memory segment allocated
	and mark the segment to be destroyed with IPC_RMID.
	Called in utils.c
*/

int remove_shm()
{
	if(shmdt(id_counter) < 0)
		DieWithError("shmdt() failed\n");
	if(shmdt(user) < 0) 
		DieWithError("shmdt() failed\n");
	if(shmdt(topic) < 0) 
		DieWithError("shmdt() failed\n");
	if(shmdt(thread) < 0)
		DieWithError("shmdt() failed\n");
	if(shmdt(message) < 0)
		DieWithError("shmdt() failed\n");

	if(shmctl(shmid_counter, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if(shmctl(shmid_auth, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if(shmctl(shmid_topics, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if(shmctl(shmid_thread, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if(shmctl(shmid_msg, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");

	return 0;
}