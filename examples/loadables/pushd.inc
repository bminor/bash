static char *dirs_doc[] = {
  "Display the list of currently remembered directories.  Directories",
  "find their way onto the list with the `pushd' command; you can get",
  "back up through the list with the `popd' command.",
  "",
  "The -l flag specifies that `dirs' should not print shorthand versions",
  "of directories which are relative to your home directory.  This means",
  "that `~/bin' might be displayed as `/homes/bfox/bin'.  The -v flag",
  "causes `dirs' to print the directory stack with one entry per line,",
  "prepending the directory name with its position in the stack.  The -p",
  "flag does the same thing, but the stack position is not prepended.",
  "The -c flag clears the directory stack by deleting all of the elements.",
  "",
  "+N   displays the Nth entry counting from the left of the list shown by",
  "     dirs when invoked without options, starting with zero.",
  "",
  "-N   displays the Nth entry counting from the right of the list shown by",
  "     dirs when invoked without options, starting with zero.",
  (char *)NULL
};

static char *pushd_doc[] = {
  "Adds a directory to the top of the directory stack, or rotates",
  "the stack, making the new top of the stack the current working",
  "directory.  With no arguments, exchanges the top two directories.",
  "",
  "+N   Rotates the stack so that the Nth directory (counting",
  "     from the left of the list shown by `dirs', starting with"
  "     zero) is at the top.",
  "",
  "-N   Rotates the stack so that the Nth directory (counting",
  "     from the right of the list shown by `dirs', starting with"
  "     zero) is at the top.",
  "",
  "-n   suppress the normal change of directory when adding directories",
  "     to the stack, so only the stack is manipulated.",
  "",
  "dir  adds DIR to the directory stack at the top, making it the",
  "     new current working directory.",
  "",
  "You can see the directory stack with the `dirs' command.",
  (char *)NULL
};

static char *popd_doc[] = {
  "Removes entries from the directory stack.  With no arguments,",
  "removes the top directory from the stack, and cd's to the new",
  "top directory.",
  "",
  "+N   removes the Nth entry counting from the left of the list",
  "     shown by `dirs', starting with zero.  For example: `popd +0'",
  "     removes the first directory, `popd +1' the second.",   
  "",
  "-N   removes the Nth entry counting from the right of the list",
  "     shown by `dirs', starting with zero.  For example: `popd -0'",
  "     removes the last directory, `popd -1' the next to last.",
  "",
  "-n   suppress the normal change of directory when removing directories",
  "     from the stack, so only the stack is manipulated.",
  "",
  "You can see the directory stack with the `dirs' command.",
  (char *)NULL
};

struct builtin pushd_struct = {
	"pushd",
	pushd_builtin,
	BUILTIN_ENABLED,
	pushd_doc,
	"pushd [+N | -N] [-n] [dir]",
	0
};

struct builtin popd_struct = {
	"popd",
	popd_builtin,
	BUILTIN_ENABLED,
	popd_doc,
	"popd [+N | -N] [-n]",
	0
};

struct builtin dirs_struct = {
	"dirs",
	dirs_builtin,
	BUILTIN_ENABLED,
	dirs_doc,
	"dirs [-clpv] [+N] [-N]",
	0
};
