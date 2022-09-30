#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

#define MAXLEN 100

int split(char *line, char buf[MAXARG][MAXLEN]);

int main(int argc, char *argv[])
{
    char buf[MAXARG][MAXLEN];
    char *p[MAXARG];
    char line[MAXLEN];
    int i, paramnum;
    for (i = 0; i < argc-1; i++)
    {
        p[i] = argv[i+1];//将原参数地址存入指针数组
    }
    if (argc < 2)
    {
        printf("Not standard input!");
        exit(0);//参数不足时退出
    }
    while(strlen(gets(line, MAXLEN)) != 0)
    {
        if (fork() == 0)
        {
            paramnum = split(line, buf);//利用split函数将读取的可能带空格的字符串分割成多个参数
            for(i = 0; i < paramnum; i++)
            {
                p[argc+i-1] = buf[i];//将参数地址分别存入指针数组
            }
            p[argc+paramnum-1] = 0;//最后一个参数为null
            exec(p[0], p);
        }
        else
        {
            wait(0);
        }
    }
    exit(0);
}

int split(char *line, char buf[MAXARG][MAXLEN])
{
    int i = 0, j = 0, k = 0, state = 1;
    for(i = 0; i < strlen(line); i++)
    {
        if(state == 1)//正常读取状态
        {
            if(line[i] == ' ' || line[i] == '\n')
            {
                buf[j++][k] = '\0';
                k = 0;
                state = 0;
            }
            else
            {
                buf[j][k++] = line[i];
            }
        }
        else//空格状态
        {
            if(line[i] != ' ')
            {
                buf[j][k++] = line[i];
                state = 1;
            }
        }
    }
    return j;
}