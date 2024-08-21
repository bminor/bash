#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#if defined (S_IFDIR) && !defined (S_ISDIR)
#define S_ISDIR(m)      (((m)&S_IFMT) == S_IFDIR)       /* directory */
#endif

main(c, v)
int	c;
char	**v;
{
	struct stat 	sb;
	int	r, fd;
	char	fbuf[32];

	r = stat("/dev/fd", &sb);
	/* test -d /dev/fd */
	if (r == -1 || S_ISDIR (sb.st_mode) == 0)
		exit (1);
	/* test -r /dev/stdin < /dev/null */
	fd = open("/dev/null", O_RDONLY, 0666);
	if (fd == -1)
		exit (2);
	if (dup2(fd, 0) == -1)
		exit (1);
	r = access("/dev/stdin", R_OK);
	if (r == -1)
		exit (1);
	exit (0);
}

	
