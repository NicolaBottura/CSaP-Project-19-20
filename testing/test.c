#include "../whiteboard.h"

int main()
{
	char buff[100];
	char s1[] = "test#1\n";
	char s2[] = "Ciao mi chiamo Nicola\n";

	strcat(s1, s2);
	printf("%s\n", s1);

	return 0;
}
