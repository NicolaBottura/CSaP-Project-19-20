#include "whiteboard.h"

#define SIZE 1000	/* !!!Provvisorio!!! */

void DieWithError(char *message);

int init_shm(int perms)
{
	if((shmid=shmget(SHMKEY, SIZE, perms)) < 0)
		DieWithError("shmget() failed\n");

	// Definire numero massimo di topics (?)
	// La struct che controlla il tutto deve essere messa in shared memory
	// Quindi la variabile che punta alla struct dovra' avere come valore 
	// il valore di ritorno di shmat

	if((shmat(shmid, NULL, 0)) < 0)
		DieWithError("shmat() failed\n");

	return 0;
}

int remove_shm()
{
	
}