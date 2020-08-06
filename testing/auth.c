#include "/home/viilz/CSaP-Project-19-20/whiteboard.h"

#define auth_length 20
#define path "/home/viilz/CSaP-Project-19-20/testing/credentials.txt"

typedef struct authentication {
	char username[auth_length];
	char password[auth_length];
	int logged;
} auth_user;
auth_user user;

int check_cred(char name[], char passwd[]);

int main(int argc, char *argv[])
{
	char name[auth_length], passwd[auth_length];

	user.logged=0;
	
	printf("Username: ");
	fgets(name, auth_length, stdin);
	printf("\nPassowrd: ");
	fgets(passwd, auth_length, stdin);
	printf("\n");

	check_cred(name, passwd);
}

int check_cred(char name[], char passwd[])
{
	FILE *fd;
	char line[256];

	/* Remove the '\n' from the user input name and password */
	int namelen=strlen(name);
	name[namelen-1]=0;
	int passlen=strlen(passwd);
	passwd[passlen-1]=0;

	if((fd=fopen(path, "r")) < 0)
		perror("open failed\n");

	while((fscanf(fd, "%s %s", user.username, user.password)) > 0)
	{
		if((strcmp(name, user.username) == 0) && ((strcmp(passwd, user.password) == 0)))
		{
			user.logged=1;
			break;
		}
		else
			continue;
	}

	/* Just print if login failed or not */
	if(user.logged == 1)
		printf("OK!\n");
	else if (user.logged == 0)
		printf("Login failed!\n");

	fclose(fd);	

	return 0;
}