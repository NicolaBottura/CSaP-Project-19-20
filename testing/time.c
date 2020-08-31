#include <stdio.h>
#include <time.h>

int main()
{
	clock_t start = clock(), diff;
	time_t delay = 10000;
	int msec;

	while(msec < delay)
	{
		diff=clock() - start;
		msec=diff * 1000 / CLOCKS_PER_SEC;
	}
	printf("%d seconds %d milliseconds", msec/1000, msec%1000);
}
