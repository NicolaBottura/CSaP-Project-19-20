#include "../whiteboard.h"

int main()
{
	int len=0, size=0;
	char buff[256];

	strcpy(buff, "1");
	len=strlen(buff);
	size=sizeof(buff);
	printf("len=%d, size=%d\n", len, size);

	return 0;
}
