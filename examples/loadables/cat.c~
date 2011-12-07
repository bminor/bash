/*
 * cat replacement
 *
 * no options - the way cat was intended
 */

#include <fcntl.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"

#ifndef errno
extern int errno;
#endif

extern char *strerror ();
extern char **make_builtin_argv ();

static int
fcopy(fd)
int	fd;
{
	char	buf[1024], *s;
	int	n, w, e;

	while (n = read(fd, buf, sizeof (buf))) {
		w = write(1, buf, n);
		if (w != n) {
			e = errno;
			write(2, "cat: write error: ", 18);
			s = strerror(e);
			write(2, s, strlen(s));
			write(2, "\n", 1);
			return 1;
		}
	}
	return 0;
}

cat_main (argc, argv)
int	argc;
char	**argv;
{
	int	i, fd, r;
	char	*s;

	if (argc == 1)
		return (fcopy(0));

	for (i = r = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '\0')
			fd = 0;
		else {
			fd = open(argv[i], O_RDONLY, 0666);
			if (fd < 0) {
				s = strerror(errno);
				write(2, "cat: cannot open ", 17);
				write(2, argv[i], strlen(argv[i]));
				write(2, ": ", 2);
				write(2, s, strlen(s));
				write(2, "\n", 1);
				continue;
			}
		}
		r = fcopy(fd);
		if (fd != 0)
			close(fd);
	}
	return (r);
}

cat_builtin(list)
WORD_LIST *list;
{
	char	**v;
	int	c, r;

	v = make_builtin_argv(list, &c);
	r = cat_main(c, v);
	free(v);

	return r;
}

char *cat_doc[] = {
	"Read each FILE and display it on the standard output.   If any",
	"FILE is `-' or if no FILE argument is given, the standard input",
	"is read.",
	(char *)0
};

struct builtin cat_struct = {
	"cat",
	cat_builtin,
	BUILTIN_ENABLED,
	cat_doc,
	"cat [-] [file ...]",
	0
};
