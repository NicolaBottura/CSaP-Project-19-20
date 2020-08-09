#include <stdio.h>
#include <stdlib.h>

int COUNT=0;

#define TEST	0

typedef struct test {
	int id;
} testing;
testing test1[10];

int main()
{
	TEST=1;
	printf("%d\n", test1[COUNT].id);
	COUNT+=1;
	test1[COUNT].id=COUNT;
	printf("%d\n", test1[COUNT].id);

	return 0;
}
