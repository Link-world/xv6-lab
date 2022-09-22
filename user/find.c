#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *dir, char *fnm);

int main(int argc, char *argv[])
{
    if (argc < 3) 
    {
        printf("Not standard input!1\n");
    }
    else 
    {
        find(argv[1], argv[2]);
    }
    exit(0);
}

void find(char *dir, char *fnm)
{
    int fd;
	char path[512];
	char *p;
    struct dirent de;
    struct stat st;
    
    if ((fd = open(dir, 0)) < 0) 
    {
        printf("find: cannot open it.\n");
        return;
    }
    if (fstat(fd, &st) < 0)
    {
        printf("find: cannot stat it.\n");
        return;
    }
	switch(st.type)
	{
	case T_FILE:
		printf("Not standard input");
		break;
	case T_DIR:
		if(strlen(dir) + 1 + DIRSIZ + 1 > sizeof path)
		{
      		printf("find: path too long\n");
      		break;
    	}
		strcpy(path, dir);
		p = path + strlen(path);
		*p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)) 
        {
            memmove(p, de.name, DIRSIZ);
      		p[DIRSIZ] = 0;
			if (de.inum == 0)
            {
				continue;
            } else if (stat(path, &st) < 0)
            {
				continue;
            } else if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
			{
				continue;
			}
			if (st.type == T_DIR)
            {
				find(path, fnm);
            } else if (strcmp(fnm, de.name) == 0)
            {
                printf("%s\n", path, st.type);
            }
        }
	}
	close(fd);
}