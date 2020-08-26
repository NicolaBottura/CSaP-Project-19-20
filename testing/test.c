#include "../whiteboard.h"

#define N 10

int main()
{
	typedef enum {UNREAD, READ} msg_status;

	msg_status status[N];
	for(int j=0; j<N; j++)
		status[j] = UNREAD;
	for(int j=0; j<N; j++)
		printf("%d\n", status[j]);

	for(int j=0; j<N; j++)
		status[j] = READ;
	for(int j=0; j<N; j++)
		printf("%d\n", status[j]);
	return 0;
}
