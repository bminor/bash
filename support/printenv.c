/* printenv -- minimal clone of BSD printenv(1).

   usage: printenv [varname]

   Chet Ramey
   chet@po.cwru.edu
*/

extern char **environ;

int
main (argc, argv) 
     int argc;
     char **argv;
{
  register char **envp, *eval;
  int len;

  argv++;
  argc--;

  /* printenv */
  if (argc == 0)
    {
      for (envp = environ; *envp; envp++)
	puts (*envp);
      exit (0);
    }

  /* printenv varname */
  len = strlen (*argv);
  for (envp = environ; *envp; envp++)
    {
      if (**argv == **envp && strncmp (*envp, *argv, len) == 0)
	{
	  eval = *envp + len;
	  /* If the environment variable doesn't have an `=', ignore it. */
	  if (*eval == '=')
	    {
	      puts (eval + 1);
	      exit (0);
	    }
	}
    }
  exit (1);
}
  
