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
		if(countright == 0)
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
				if (countleft == 0)
				{
					prime = i;
					printf("prime %d \n", i);
					countleft = 1;
				}
				if (i % prime != 0)
				{
					if (countright == 0)
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
				if (countright != 0)
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
	else
	{
		close(rightp);
	}
	wait(0);
	exit(0);
}