/* parser.h -- Everything you wanted to know about the parser, but were
   afraid to ask. */
#if !defined (_PARSER_H_)
#  define _PARSER_H_

#  include "command.h"
#  include "input.h"

/* Definition of the delimiter stack.  Needed by parse.y and bashhist.c. */
struct dstack {
/* DELIMITERS is a stack of the nested delimiters that we have
   encountered so far. */
  char *delimiters;

/* Offset into the stack of delimiters. */
  int delimiter_depth;

/* How many slots are allocated to DELIMITERS. */
  int delimiter_space;
};

#endif /* _PARSER_H_ */
