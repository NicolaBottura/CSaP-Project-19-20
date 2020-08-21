#include "whiteboard.h"

#define SIZE 1000	/* !!!Provvisorio!!! */

int init_shm(int perms)
{
	/* Shared memory for both structures and an id counter for the clients */
	if((shmid_counter=shmget(SHMKEY_C, sizeof(int *), perms)) < 0)
		DieWithError("shmget()#1 failed\n");
	if((shmid_auth=shmget(SHMKEY_A, SIZE, perms)) < 0)
		DieWithError("shmget()#2 failed\n");
	if((shmid_topics=shmget(SHMKEY_T, SIZE, perms)) < 0)
		DieWithError("shmget()#3 failed\n");

	// Definire numero massimo di topics (?)
	// La struct che controlla il tutto deve essere messa in shared memory
	// Quindi la variabile che punta alla struct dovra' avere come valore 
	// il valore di ritorno di shmat

	if((id_counter=shmat(shmid_counter, NULL, 0)) < 0)
		DieWithError("shmat()#1 failed\n");
	if((user=(auth_user *)shmat(shmid_auth, NULL, 0)) < 0)
		DieWithError("shmat()#2 failed\n");
	if((topic=(topics *)shmat(shmid_topics, NULL, 0)) < 0)
		DieWithError("shmat()#3 failed\n");

	return 0;
}

int remove_shm()
{
	if(shmdt(user) < 0) 
		DieWithError("shmdt() failed\n");
	if(shmdt(topic) < 0) 
		DieWithError("shmdt() failed\n");
	if(shmdt(id_counter) < 0) 
		DieWithError("shmdt() failed\n");

	if (shmctl(shmid_auth, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if (shmctl(shmid_topics, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if (shmctl(shmid_counter, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");

	return 0;
}