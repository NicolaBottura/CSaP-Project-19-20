#include "../whiteboard.h"

#define N 10

int main()
{
	int a[N] = {1,3,5,7,23,674,32,4,0,332};
	int x = 45;

	for(int j=0; j<N; j++)
	{
		if(a[j] != x && j == N-1)
		{
			printf("Not found - last item: %d\n", a[j]);
			return -1;
		}
		else if(a[j] == x)
			break;
	}
	
	printf("I'm out!\n");
		
	return 0;
}
