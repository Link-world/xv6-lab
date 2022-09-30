#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv)
{
	int i;
	int prime;
	int countleft = 0;
	int countright = 0;
	int p[2];
	int leftp = 0;
	int rightp = 0;
	for (i = 2; i <= 35; i++)
	{
		if(countright == 0)//第一层向此后多层传输2至35
		{
            pipe(p);
			if (fork() == 0)
			{
				close(p[1]);
				leftp = p[0];
				break;
			}
			else
			{
				close(p[0]);
				rightp = p[1];
				countright = 1;
			}
		}
		write(rightp, &i, sizeof(int));
	}
	if (i != 36)
	{
		while (1)
		{
			if (read(leftp, &i, sizeof(int)) != 0)
			{
				if (countleft == 0)//第一次创建的层将传过来的第一个数作为该层筛选所用质数
				{
					prime = i;
					printf("prime %d \n", i);
					countleft = 1;
				}
				if (i % prime != 0)//当出现不可被质数整除时
				{
					if (countright == 0)//如果第一次创建后续层，将建立新的管道
					{
						pipe(p);
						if (fork() == 0)
						{
							close(p[1]);
							countleft = 0;
							leftp = p[0];
							continue;
						}
						else
						{
							close(p[0]);
							rightp = p[1];
							countright = 1;
						}
					}
					write(rightp, &i, sizeof(int));
				}
			}
			else
			{
				if (countright != 0)//如果创建了后续层，则需要将它的右侧端口关闭
				{
					close(rightp);
					break;
				}
				else
				{
					exit(0);
				}
			}
		}
	}
	else//第一层的右侧端口关闭
	{
		close(rightp);
	}
	wait(0);//等待子进程
	exit(0);
}