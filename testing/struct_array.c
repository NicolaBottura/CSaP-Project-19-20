#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct test {
	int id;
	char nome[20];
} testing;
testing *test1;

int count;

int main()
{
	count++;
	test1=realloc(test1, sizeof *test1);
	test1[count].id=3;
	strcpy(test1[count].nome, "Nicola");

	count++;
	test1[count].id=55;
	strcpy(test1[count].nome, "Giorgia");

	count++;
	test1[count].id=109;
	strcpy(test1[count].nome, "Thanos");

	for(int j=1; j<=count; j++)
		printf("%s\n", test1[j].nome);
}
