#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#ifndef errno
extern int errno;
#endif
main()
{
	int	x;

	x = waitpid(-1, (int *)0, WNOHANG|WCONTINUED);
	if (x == -1 && errno == ECHILD)
		exit (0);
	else
		exit (1);
}
