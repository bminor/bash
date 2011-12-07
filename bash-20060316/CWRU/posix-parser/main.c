#include <stdio.h>
#include "shell.h"

COMMAND *global_command;
int last_command_exit_value;
int interrupt_state;
int interactive = 1;
int eof_encountered = 0;
int exit_immediately_on_error = 1;

char *the_current_maintainer = "chet";

char *shell_name = "posix";

void
throw_to_top_level()
{
}

char *
base_pathname(s)
char	*s;
{
	return s;
}

char *
strerror(s)
int	s;
{
	return ("error");
}

parse_command ()
{
  extern int need_here_doc, current_command_line_count;
  extern REDIRECT *redirection_needing_here_doc;
  int r;

  need_here_doc = 0;
  redirection_needing_here_doc = (REDIRECT *)NULL;

  current_command_line_count = 0;
  r = yyparse ();

  if (need_here_doc)
    make_here_document (redirection_needing_here_doc);
  need_here_doc = 0;

  return (r);
}

main(argc, argv)
int	argc;
char	**argv;
{
	with_input_from_stdin();

	if (parse_command () == 0) {
		printf ("legal command in the Posix shell\n");
		exit (0);
	} else {
		printf ("illegal\n");
		exit (1);
	}
}

char *
string_quote_removal (s)
{
	return (savestring (s));
}

assignment (string)
     char *string;
{
  register int c, index = 0;

  c = string[index];

  if (!isletter (c) && c != '_')
    return (0);

  while (c = string[index])
    {
      /* The following is safe.  Note that '=' at the start of a word
         is not an assignment statement. */
      if (c == '=')
        return (index);

      if (!isletter (c) && !digit (c) && c != '_')
        return (0);

      index++;
    }
  return (0);
}
