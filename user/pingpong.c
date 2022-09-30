#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	int pid;
	int p[2];
	int pAnother[2];//两个管道分别用于两个方向传输
	char b[1];
	char bAnother[1];
	pipe(p);
    pipe(pAnother);
	if (fork() == 0) 
	{
		close(p[1]);//关闭写端
		close(pAnother[0]);//关闭读端
		if (read(p[0], b, sizeof(char)) == 0)//从一个管道读
		{
			printf("Error\n");
		}
	    else 
		{
			pid = getpid();
			printf("%d: received ping\n", pid);
			write(pAnother[1], b, sizeof(char));//向另一个管道写
		}
		close(p[0]);
		close(pAnother[1]);
		exit(0);
	}	
	else
	{
		close(p[0]);//关闭读端
		close(pAnother[1]);//关闭写端
		write(p[1], b, sizeof(char));//写入管道
        wait(0);
		if (read(pAnother[0], bAnother, sizeof(char)) != 0 && *b == *bAnother)//检测是否传回正确
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
