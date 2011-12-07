/*
 * Originally from
 *	http://www.excessus.demon.co.uk/misc-hacks/index.html#xtitle
 */

/*
 * Made into a loadable builtin by chet@po.cwru.edu.
 */

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <termios.h>

#ifdef BASH_BUILTIN
#include "shell.h"
#include "builtins.h"
#include "bashgetopt.h"
#endif

#ifdef BASH_BUILTIN
int xtitle_builtin(WORD_LIST *list)
#else
int main(int argc, char *argv[])
#endif
{
  int query = 0;
  int fd;
  int openned = 0;

#ifdef BASH_BUILTIN
  reset_internal_getopt();
#endif
  for (;;) {
#ifdef BASH_BUILTIN
    int i;
    i = internal_getopt(list, "q");
#else
    int i = getopt(argc, argv, "q");
#endif
    if (i < 0)
      break;
    switch (i) {
      case 'q':
	query = 1;
	break;
      default:
#ifdef BASH_BUILTIN
	builtin_usage();
#else
	fprintf(stderr, "usage: xtitle [-q] [string]\n");
#endif
	return (1);
    }
  }

#ifdef BASH_BUILTIN
  if (!query && loptend == 0) {
#else
  if (!query && optind == argc) {
#endif
    fprintf(stderr, "xtitle: no string to set\n");
    return (1);
  }

  {
    char *t = getenv("TERM");
    if (!t || strncmp(t, "xterm", 5))
      return (0);
  }

  if (isatty(0))
    fd = 0;
  else {
    fd = open("/dev/tty", O_RDWR);
    if (fd < 0) {
      fprintf(stderr, "xtitle: couldn't open terminal: %s", strerror(errno));
      return (1);
    }
    openned = 1;
  }

  if (!query) {
#ifdef BASH_BUILTIN
    WORD_LIST *l = loptend;
    char sp = ' ';
    write(fd, "\33]0;", 4);
    while (l) {
      write(fd, l->word->word, strlen(l->word->word));
      if (l->next)
	write(fd, &sp, 1);
      l = l->next;
    }
    write(fd, "\33\\", 2);
#else
    int i;
    char sp = ' ';
    write(fd, "\33]0;", 4);
    for (i = optind; i < argc; i++) {
      write(fd, argv[i], strlen(argv[i]));
      if (i < argc - 1)
	write(fd, &sp, 1);
    }
    write(fd, "\33\\", 2);
#endif
  } else {
    struct termios o, n;
    char hack;
    int state = 0;

    tcgetattr(fd, &o);
    n = o;
    n.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
		   |INLCR|IGNCR|ICRNL|IXON);
    n.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    n.c_cflag &= ~(CSIZE|PARENB);
    n.c_cflag |= CS8;
    tcsetattr(fd, TCSAFLUSH, &n);
    write(fd, "\33[21t", 5);

    while (state != -1) {
      if (read(fd, &hack, 1) < 1)
	break;
      switch (state) {
	case 0:
	  if (hack == '\33') state = 1;
	  break;
	case 1:
	  if (hack == ']') state = 2; else state = 0;
	  break;
	case 2:
	  if (hack == 'l') state = 3; else state = 0;
	  break;
	case 3:
	  if (hack == '\33') state = 4; else putchar(hack);
	  break;
	case 4:
	  if (hack == '\\') { state = -1; putchar('\n'); }
	  else { putchar('\33'); putchar(hack); state = 3; }
	  break;
      }
    }

    tcsetattr(fd, TCSAFLUSH, &o);
  }

  if (openned)
    close(fd);

  return (0);
}


#ifdef BASH_BUILTIN

static char *xtitle_doc[] = {
  "Either set or read the title of the current xterm window.  With the",
  "-q option, writes the current xterm title to standard output.  Without",
  "the -q option, sets the xterm title to be the arguments given,",
  "separated by space characters.  [By Mark Wooding, mdw@nsict.org]",
  0
};

struct builtin xtitle_struct = {
  "xtitle",
  xtitle_builtin,
  BUILTIN_ENABLED,
  xtitle_doc,
  "xtitle [-q] [arguments]",
  0
};

#endif
