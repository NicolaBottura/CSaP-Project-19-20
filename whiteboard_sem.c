#include "whiteboard.h"

int get_sem(int perms)
{
	if((semid=semget(SEMKEY, NUMSEM, perms)) < 0)
		DieWithError("semget() failed\n");

	return 0;
}

int init_sem()
{
// 	for(int i = 0; i < NUMSEM; i++)
	if(semctl(semid, 0, SETVAL, AUTH_CS) < 0)
		DieWithError("semctl()init failed\n");
  
	return 0;
}

int remove_sem()
{
	if(semctl(semid, 0, IPC_RMID) < 0)
		DieWithError("semctl()remove - failed\n");
}

int p(int semnum)
{
	struct sembuf p_buf;

	p_buf.sem_num = 0; 		// Sarebbe il semnum - se avessi un array di semafori in questo direi quale modificare
	p_buf.sem_op  = -1;
	p_buf.sem_flg = 0;

	if(semop(semid, &p_buf, 1) == -1)
		DieWithError("semop()p failed\n");
	else
		return 0;
}

int v(int semnum)
{
	struct sembuf v_buf;

	v_buf.sem_num = 0;
	v_buf.sem_op  = 1;
	v_buf.sem_flg = 0;

	if(semop(semid, &v_buf, 1) == -1)
		DieWithError("semop()v failed\n");
	else 
		return 0;
}
