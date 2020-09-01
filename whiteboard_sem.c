#include "whiteboard.h"

int get_sem(int perms)
{
	if((semid=semget(SEMKEY, NUMSEM, perms)) < 0)		/* get a SYSV sempahore set identifier - the parameters are defined in whiteboard.h */
		DieWithError("semget() failed\n");

	return 0;
}

int init_sem(int semvals[])
{
 	for(int i = 0; i < NUMSEM; i++)						/* For each sempahore */
		if(semctl(semid, i, SETVAL, semvals[i]) < 0)	/* set the value for i-th sempahore */
			DieWithError("semctl()init failed\n");
  
	return 0;
}

int remove_sem()
{
	if(semctl(semid, 0, IPC_RMID) < 0)					/* Remove the semaphore set */
		DieWithError("semctl()remove - failed\n");

	return 0;
}

int p(int semnum)										/* Sempahore p-operation */
{
	struct sembuf p_buf;

	p_buf.sem_num = semnum;								/* Semaphore number - SEMAUTH/SEMTOPICS */
	p_buf.sem_op  = -1;									/* Semaphore operation */
	p_buf.sem_flg = 0;

	if(semop(semid, &p_buf, 1) == -1)					/* Perform an operation on selected semaphore */
		DieWithError("semop()p failed\n");
	else
		return 0;
}

int v(int semnum)										/* Semaphore v-operation */
{
	struct sembuf v_buf;

	v_buf.sem_num = semnum;								/* Semaphore number - SEMAUTH/SEMTOPICS */		
	v_buf.sem_op  = 1;									/* Semaphore operation */
	v_buf.sem_flg = 0;

	if(semop(semid, &v_buf, 1) == -1)					/* Perform an operation on selected semaphore */
		DieWithError("semop()v failed\n");
	else 
		return 0;
}
