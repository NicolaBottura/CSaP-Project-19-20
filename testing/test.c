#include "../whiteboard.h"

int main()
{
	int *a;
	int size = 3;
	int b[4];

	printf("size: %d\n", sizeof(b));
		
	a=(int *)calloc(size, sizeof(int));
	a[0]=3;
	a[1]=2;
	a[2]=5;
	for(int j=0; j<3; j++)
		printf("%d\n", a[j]);

	free(a[1]);

	for(int j=0; j<3; j++)
		printf("%d\n", a[j]);
	
	return 0;
}
