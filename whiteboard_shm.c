#include "whiteboard.h"

#define SIZE 1000	/* !!!Provvisorio!!! */

int init_shm(int perms)
{
	/* Shared memory for both structures */
	if((shmid_auth=shmget(SHMKEY_A, SIZE, perms)) < 0)
		DieWithError("shmget() failed\n");
	if((shmid_topics=shmget(SHMKEY_T, SIZE, perms)) < 0)
		DieWithError("shmget() failed\n");

	// Definire numero massimo di topics (?)
	// La struct che controlla il tutto deve essere messa in shared memory
	// Quindi la variabile che punta alla struct dovra' avere come valore 
	// il valore di ritorno di shmat

	if((user=shmat(shmid_auth, NULL, 0)) < 0)
		DieWithError("shmat() failed\n");

	if((topic=shmat(shmid_topics, NULL, 0)) < 0)
		DieWithError("shmat() failed\n");

	return 0;
}

int remove_shm()
{
	if(shmdt(user) < 0) 
		DieWithError("shmdt() failed\n");
	if(shmdt(topic) < 0) 
		DieWithError("shmdt() failed\n");


	if (shmctl(shmid_auth, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");
	if (shmctl(shmid_topics, IPC_RMID, NULL) < 0)
		DieWithError("shmctl() failed\n");

	return 0;
}