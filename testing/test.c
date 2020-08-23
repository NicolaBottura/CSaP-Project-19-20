#include "../whiteboard.h"

int main()
{
	char buff[100];
	char s1[] = "test#1\n";
	char s2[] = "Ciao mi chiamo Nicola\n";
	int a[3];

	printf("sizeof array[3]: %d\n", sizeof(a));
	strcat(s1, s2);
	printf("%s\n", s1);

	return 0;
}
