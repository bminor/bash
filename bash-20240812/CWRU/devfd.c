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
	/* test -r /dev/fd/0 */
	r = access ("/dev/fd/0", R_OK);
	if (r == -1)
		exit (1);
	/* exec 3</dev/null */
	fd = open("/dev/null", O_RDONLY, 0666);
	if (fd == -1)
		exit (2);
	if (fd != 3 && (dup2(fd, 3) == -1))
		exit (1);
	/* test -r /dev/fd/3 */
	r = access("/dev/fd/3", R_OK);
	if (r == -1)
		exit (1);
	exit (0);
}

	
