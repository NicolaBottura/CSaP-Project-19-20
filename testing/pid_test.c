#include "../whiteboard.h"

int main()
{
	int pid=0, c=0;
	int pids[5];
	pid_t pp;

	if((pp=fork()) == 0)
	{
		printf("I am PID #%d\n", getpid());
		pids[c]=getpid();
		c++;
		exit(0);
	}

	printf("I am PID #%d\n", getpid());
	pids[c]=getpid();

	for(int j=0; j < 5; j++)
	{
		if(pids[j]==getpid())
			printf("%d\n", pids[j]);
	}
	return 0;
}
