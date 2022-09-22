#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int xgets(char *buf);

int main(int argc, char *argv[])
{
    char line[MAXARG];
    char *p[argc + 2];
    int i;
    for (i = 0; i < argc; i++)
    {
        p[i] = argv[i];
    }
    if (argc < 2)
    {
        printf("Not standard input!");
        exit(0);
    }
    while(xgets(line) != 0)
    {
        if (fork() == 0)
        {
            p[argc] = line;
            p[argc + 1] = 0;
            exec(argv[1], &(p[1]));
        }
        else
        {
            wait(0);
        }
    }
    exit(0);
}

int xgets(char *buf)
{
    int i = 0, cc;
    char c;

    while(1)
    {
        cc = read(0, &c, 1);
        if(cc < 1)
            return 0;
        buf[i++] = c;
        if(c == '\n' || c == '\r')
        break;
    }
    buf[i-1] = '\0';
    return 1;
}