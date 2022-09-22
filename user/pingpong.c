#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	int pid;
	int p[2];
	int pAnother[2];
	char b[1];
	char bAnother[1];
	pipe(p);
    pipe(pAnother);
	if (fork() == 0) 
	{
		close(p[1]);
		close(pAnother[0]);
		if (read(p[0], b, sizeof(char)) == 0)
		{
			printf("Error\n");
		}
	       	else 
		{
			pid = getpid();
			printf("%d: received ping\n", pid);
			write(pAnother[1], b, sizeof(char));
		}
		close(p[0]);
		close(pAnother[1]);
		exit(0);
	}	
	else
	{
		close(p[0]);
		close(pAnother[1]);
		write(p[1], b, sizeof(char));
        wait(0);
		if (read(pAnother[0], bAnother, sizeof(char)) != 0 && *b == *bAnother)
		{
			pid = getpid();
			printf("%d: received pong\n", pid);
		}
		else 
		{
			printf("Error!\n");
		}
		close(p[1]);
		close(pAnother[0]);
		exit(0);	
	}
}
