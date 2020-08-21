#include "../whiteboard.h"

int main()
{
	FILE *fd;
	int n=0;
	char buff[BUFFSIZE];
	char *val;

	fd=fopen("file.txt", "rw");

	while((n=fread(buff, sizeof(char), BUFFSIZE, fd)) > 0)
	{
		if((val=strstr(buff, "Nicola")) != NULL)
			for(int i=0; val[i] != '\0'; i++)
			{
				val[i]="";
				fprintf(fd, "%s", val[i]);
			} 
	}
}

