/* A Bison parser, made from /usr/homes/chet/src/bash/src/parse.y
   by GNU bison 1.34.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	IF	257
# define	THEN	258
# define	ELSE	259
# define	ELIF	260
# define	FI	261
# define	CASE	262
# define	ESAC	263
# define	FOR	264
# define	SELECT	265
# define	WHILE	266
# define	UNTIL	267
# define	DO	268
# define	DONE	269
# define	FUNCTION	270
# define	COND_START	271
# define	COND_END	272
# define	COND_ERROR	273
# define	IN	274
# define	BANG	275
# define	TIME	276
# define	TIMEOPT	277
# define	WORD	278
# define	ASSIGNMENT_WORD	279
# define	NUMBER	280
# define	ARITH_CMD	281
# define	ARITH_FOR_EXPRS	282
# define	COND_CMD	283
# define	AND_AND	284
# define	OR_OR	285
# define	GREATER_GREATER	286
# define	LESS_LESS	287
# define	LESS_AND	288
# define	LESS_LESS_LESS	289
# define	GREATER_AND	290
# define	SEMI_SEMI	291
# define	LESS_LESS_MINUS	292
# define	AND_GREATER	293
# define	LESS_GREATER	294
# define	GREATER_BAR	295
# define	yacc_EOF	296

#line 21 "/usr/homes/chet/src/bash/src/parse.y"

#include "config.h"

#include "bashtypes.h"
#include "bashansi.h"

#include "filecntl.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (HAVE_LOCALE_H)
#  include <locale.h>
#endif

#include <stdio.h>
#include "chartypes.h"
#include <signal.h>

#include "memalloc.h"

#define NEED_STRFTIME_DECL	/* used in externs.h */

#include "shell.h"
#include "trap.h"
#include "flags.h"
#include "parser.h"
#include "mailcheck.h"
#include "test.h"
#include "builtins/common.h"
#include "builtins/builtext.h"

#include "shmbutil.h"

#if defined (READLINE)
#  include "bashline.h"
#  include <readline/readline.h>
#endif /* READLINE */

#if defined (HISTORY)
#  include "bashhist.h"
#  include <readline/history.h>
#endif /* HISTORY */

#if defined (JOB_CONTROL)
#  include "jobs.h"
#endif /* JOB_CONTROL */

#if defined (ALIAS)
#  include "alias.h"
#endif /* ALIAS */

#if defined (PROMPT_STRING_DECODE)
#  ifndef _MINIX
#    include <sys/param.h>
#  endif
#  include <time.h>
#  if defined (TM_IN_SYS_TIME)
#    include <sys/types.h>
#    include <sys/time.h>
#  endif /* TM_IN_SYS_TIME */
#  include "maxpath.h"
#endif /* PROMPT_STRING_DECODE */

#define RE_READ_TOKEN	-99
#define NO_EXPANSION	-100

#ifdef DEBUG
#  define YYDEBUG 1
#else
#  define YYDEBUG 0
#endif

#if defined (HANDLE_MULTIBYTE)
#  define last_shell_getc_is_singlebyte \
	((shell_input_line_index > 1) \
		? shell_input_line_property[shell_input_line_index - 1] \
		: 1)
#  define MBTEST(x)	((x) && last_shell_getc_is_singlebyte)
#else
#  define last_shell_getc_is_singlebyte	1
#  define MBTEST(x)	((x))
#endif

#if defined (EXTENDED_GLOB)
extern int extended_glob;
#endif

extern int eof_encountered;
extern int no_line_editing, running_under_emacs;
extern int current_command_number;
extern int sourcelevel;
extern int posixly_correct;
extern int last_command_exit_value;
extern int interrupt_immediately;
extern char *shell_name, *current_host_name;
extern char *dist_version;
extern int patch_level;
extern int dump_translatable_strings, dump_po_strings;
extern sh_builtin_func_t *last_shell_builtin, *this_shell_builtin;
#if defined (BUFFERED_INPUT)
extern int bash_input_fd_changed;
#endif

extern int errno;
/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

#ifdef DEBUG
static void debug_parser __P((int));
#endif

static int yy_getc __P((void));
static int yy_ungetc __P((int));

#if defined (READLINE)
static int yy_readline_get __P((void));
static int yy_readline_unget __P((int));
#endif

static int yy_string_get __P((void));
static int yy_string_unget __P((int));
static int yy_stream_get __P((void));
static int yy_stream_unget __P((int));

static int shell_getc __P((int));
static void shell_ungetc __P((int));
static void discard_until __P((int));

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
static void push_string __P((char *, int, alias_t *));
static void pop_string __P((void));
static void free_string_list __P((void));
#endif

static char *read_a_line __P((int));

static int reserved_word_acceptable __P((int));
static int yylex __P((void));
static int alias_expand_token __P((char *));
static int time_command_acceptable __P((void));
static int special_case_tokens __P((char *));
static int read_token __P((int));
static char *parse_matched_pair __P((int, int, int, int *, int));
#if defined (ARRAY_VARS)
static char *parse_compound_assignment __P((int *));
#endif
#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
static int parse_dparen __P((int));
static int parse_arith_cmd __P((char **));
#endif
#if defined (COND_COMMAND)
static void cond_error __P((void));
static COND_COM *cond_expr __P((void));
static COND_COM *cond_or __P((void));
static COND_COM *cond_and __P((void));
static COND_COM *cond_term __P((void));
static int cond_skip_newlines __P((void));
static COMMAND *parse_cond_command __P((void));
#endif
#if defined (ARRAY_VARS)
static int token_is_assignment __P((char *, int));
static int token_is_ident __P((char *, int));
#endif
static int read_token_word __P((int));
static void discard_parser_constructs __P((int));

static char *error_token_from_token __P((int));
static char *error_token_from_text __P((void));
static void print_offending_line __P((void));
static void report_syntax_error __P((char *));

static void handle_eof_input_unit __P((void));
static void prompt_again __P((void));
#if 0
static void reset_readline_prompt __P((void));
#endif
static void print_prompt __P((void));

#if defined (HISTORY)
char *history_delimiting_chars __P((void));
#endif

#if defined (HANDLE_MULTIBYTE)
static void set_line_mbstate __P((void));
static char *shell_input_line_property = NULL;
#else
#  define set_line_mbstate()
#endif

extern int yyerror __P((const char *));

#ifdef DEBUG
extern int yydebug;
#endif

/* Default prompt strings */
char *primary_prompt = PPROMPT;
char *secondary_prompt = SPROMPT;

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* Non-zero means we expand aliases in commands. */
int expand_aliases = 0;

/* If non-zero, the decoded prompt string undergoes parameter and
   variable substitution, command substitution, arithmetic substitution,
   string expansion, process substitution, and quote removal in
   decode_prompt_string. */
int promptvars = 1;

/* The decoded prompt string.  Used if READLINE is not defined or if
   editing is turned off.  Analogous to current_readline_prompt. */
static char *current_decoded_prompt;

/* The number of lines read from input while creating the current command. */
int current_command_line_count;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
static REDIRECT *redir_stack[10];
int need_here_doc;

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
static char *shell_input_line = (char *)NULL;
static int shell_input_line_index;
static int shell_input_line_size;	/* Amount allocated for shell_input_line. */
static int shell_input_line_len;	/* strlen (shell_input_line) */

/* Either zero or EOF. */
static int shell_input_line_terminator;

/* The line number in a script on which a function definition starts. */
static int function_dstart;

/* The line number in a script on which a function body starts. */
static int function_bstart;

/* The line number in a script at which an arithmetic for command starts. */
static int arith_for_lineno;

static REDIRECTEE redir;

#line 276 "/usr/homes/chet/src/bash/src/parse.y"
#ifndef YYSTYPE
typedef union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
} yystype;
# define YYSTYPE yystype
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		301
#define	YYFLAG		-32768
#define	YYNTBASE	54

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 296 ? yytranslate[x] : 90)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      44,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    42,     2,
      52,    53,     2,     2,     2,    49,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    43,
      48,     2,    47,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,    46,    51,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    45
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    15,    18,    21,
      25,    29,    32,    36,    39,    43,    46,    50,    53,    57,
      60,    64,    67,    71,    74,    78,    81,    85,    88,    92,
      95,    99,   102,   106,   109,   112,   116,   118,   120,   122,
     124,   127,   129,   132,   134,   136,   139,   141,   143,   145,
     151,   157,   159,   161,   163,   165,   167,   169,   171,   178,
     185,   193,   201,   212,   223,   233,   243,   251,   259,   265,
     271,   278,   285,   293,   301,   312,   323,   330,   338,   345,
     351,   358,   363,   365,   368,   372,   378,   386,   393,   397,
     399,   403,   408,   415,   421,   423,   426,   431,   436,   442,
     448,   451,   455,   457,   461,   464,   466,   469,   473,   477,
     481,   486,   491,   496,   501,   506,   508,   510,   512,   514,
     516,   518,   519,   522,   524,   527,   530,   535,   540,   544,
     548,   550,   552,   555,   558,   562,   566,   571,   573,   575
};
static const short yyrhs[] =
{
      85,    82,     0,    44,     0,     1,    44,     0,    45,     0,
      24,     0,    55,    24,     0,    47,    24,     0,    48,    24,
       0,    26,    47,    24,     0,    26,    48,    24,     0,    32,
      24,     0,    26,    32,    24,     0,    33,    24,     0,    26,
      33,    24,     0,    35,    24,     0,    26,    35,    24,     0,
      34,    26,     0,    26,    34,    26,     0,    36,    26,     0,
      26,    36,    26,     0,    34,    24,     0,    26,    34,    24,
       0,    36,    24,     0,    26,    36,    24,     0,    38,    24,
       0,    26,    38,    24,     0,    36,    49,     0,    26,    36,
      49,     0,    34,    49,     0,    26,    34,    49,     0,    39,
      24,     0,    26,    40,    24,     0,    40,    24,     0,    41,
      24,     0,    26,    41,    24,     0,    24,     0,    25,     0,
      56,     0,    56,     0,    58,    56,     0,    57,     0,    59,
      57,     0,    59,     0,    61,     0,    61,    58,     0,    66,
       0,    62,     0,    65,     0,    12,    79,    14,    79,    15,
       0,    13,    79,    14,    79,    15,     0,    64,     0,    69,
       0,    68,     0,    70,     0,    71,     0,    72,     0,    63,
       0,    10,    24,    84,    14,    79,    15,     0,    10,    24,
      84,    50,    79,    51,     0,    10,    24,    43,    84,    14,
      79,    15,     0,    10,    24,    43,    84,    50,    79,    51,
       0,    10,    24,    84,    20,    55,    83,    84,    14,    79,
      15,     0,    10,    24,    84,    20,    55,    83,    84,    50,
      79,    51,     0,    10,    24,    84,    20,    83,    84,    14,
      79,    15,     0,    10,    24,    84,    20,    83,    84,    50,
      79,    51,     0,    10,    28,    83,    84,    14,    79,    15,
       0,    10,    28,    83,    84,    50,    79,    51,     0,    10,
      28,    14,    79,    15,     0,    10,    28,    50,    79,    51,
       0,    11,    24,    84,    14,    78,    15,     0,    11,    24,
      84,    50,    78,    51,     0,    11,    24,    43,    84,    14,
      78,    15,     0,    11,    24,    43,    84,    50,    78,    51,
       0,    11,    24,    84,    20,    55,    83,    84,    14,    78,
      15,     0,    11,    24,    84,    20,    55,    83,    84,    50,
      78,    51,     0,     8,    24,    84,    20,    84,     9,     0,
       8,    24,    84,    20,    76,    84,     9,     0,     8,    24,
      84,    20,    74,     9,     0,    24,    52,    53,    84,    67,
       0,    16,    24,    52,    53,    84,    67,     0,    16,    24,
      84,    67,     0,    61,     0,    61,    58,     0,    52,    79,
      53,     0,     3,    79,     4,    79,     7,     0,     3,    79,
       4,    79,     5,    79,     7,     0,     3,    79,     4,    79,
      73,     7,     0,    50,    79,    51,     0,    27,     0,    17,
      29,    18,     0,     6,    79,     4,    79,     0,     6,    79,
       4,    79,     5,    79,     0,     6,    79,     4,    79,    73,
       0,    75,     0,    76,    75,     0,    84,    77,    53,    79,
       0,    84,    77,    53,    84,     0,    84,    52,    77,    53,
      79,     0,    84,    52,    77,    53,    84,     0,    75,    37,
       0,    76,    75,    37,     0,    24,     0,    77,    46,    24,
       0,    84,    80,     0,    78,     0,    84,    81,     0,    81,
      44,    84,     0,    81,    42,    84,     0,    81,    43,    84,
       0,    81,    30,    84,    81,     0,    81,    31,    84,    81,
       0,    81,    42,    84,    81,     0,    81,    43,    84,    81,
       0,    81,    44,    84,    81,     0,    87,     0,    44,     0,
      45,     0,    44,     0,    43,     0,    45,     0,     0,    84,
      44,     0,    86,     0,    86,    42,     0,    86,    43,     0,
      86,    30,    84,    86,     0,    86,    31,    84,    86,     0,
      86,    42,    86,     0,    86,    43,    86,     0,    87,     0,
      88,     0,    21,    88,     0,    89,    88,     0,    89,    21,
      88,     0,    21,    89,    88,     0,    88,    46,    84,    88,
       0,    60,     0,    22,     0,    22,    23,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   326,   335,   342,   357,   367,   369,   373,   378,   383,
     388,   393,   398,   403,   409,   415,   420,   425,   430,   435,
     440,   445,   450,   455,   460,   465,   472,   479,   484,   489,
     494,   499,   504,   509,   514,   519,   526,   528,   530,   534,
     538,   549,   551,   555,   557,   559,   575,   579,   581,   583,
     585,   587,   589,   591,   593,   595,   597,   599,   603,   605,
     607,   609,   611,   613,   615,   617,   621,   623,   625,   627,
     631,   635,   639,   643,   647,   651,   657,   659,   661,   665,
     668,   671,   676,   678,   709,   716,   718,   720,   725,   729,
     733,   737,   739,   741,   745,   746,   750,   752,   754,   756,
     760,   761,   765,   767,   776,   784,   785,   791,   792,   799,
     803,   805,   807,   814,   816,   818,   822,   823,   826,   827,
     828,   831,   832,   841,   847,   856,   864,   866,   868,   875,
     878,   882,   884,   889,   894,   899,   906,   909,   913,   915
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "IF", "THEN", "ELSE", "ELIF", "FI", "CASE", 
  "ESAC", "FOR", "SELECT", "WHILE", "UNTIL", "DO", "DONE", "FUNCTION", 
  "COND_START", "COND_END", "COND_ERROR", "IN", "BANG", "TIME", "TIMEOPT", 
  "WORD", "ASSIGNMENT_WORD", "NUMBER", "ARITH_CMD", "ARITH_FOR_EXPRS", 
  "COND_CMD", "AND_AND", "OR_OR", "GREATER_GREATER", "LESS_LESS", 
  "LESS_AND", "LESS_LESS_LESS", "GREATER_AND", "SEMI_SEMI", 
  "LESS_LESS_MINUS", "AND_GREATER", "LESS_GREATER", "GREATER_BAR", "'&'", 
  "';'", "'\\n'", "yacc_EOF", "'|'", "'>'", "'<'", "'-'", "'{'", "'}'", 
  "'('", "')'", "inputunit", "word_list", "redirection", 
  "simple_command_element", "redirection_list", "simple_command", 
  "command", "shell_command", "for_command", "arith_for_command", 
  "select_command", "case_command", "function_def", "function_body", 
  "subshell", "if_command", "group_command", "arith_command", 
  "cond_command", "elif_clause", "case_clause", "pattern_list", 
  "case_clause_sequence", "pattern", "list", "compound_list", "list0", 
  "list1", "simple_list_terminator", "list_terminator", "newline_list", 
  "simple_list", "simple_list1", "pipeline_command", "pipeline", 
  "timespec", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    54,    54,    54,    54,    55,    55,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    57,    57,    57,    58,
      58,    59,    59,    60,    60,    60,    60,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    62,    62,
      62,    62,    62,    62,    62,    62,    63,    63,    63,    63,
      64,    64,    64,    64,    64,    64,    65,    65,    65,    66,
      66,    66,    67,    67,    68,    69,    69,    69,    70,    71,
      72,    73,    73,    73,    74,    74,    75,    75,    75,    75,
      76,    76,    77,    77,    78,    79,    79,    80,    80,    80,
      81,    81,    81,    81,    81,    81,    82,    82,    83,    83,
      83,    84,    84,    85,    85,    85,    86,    86,    86,    86,
      86,    87,    87,    87,    87,    87,    88,    88,    89,    89
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     2,     1,     2,     1,     1,     2,     2,     2,     3,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     2,     3,     1,     1,     1,     1,
       2,     1,     2,     1,     1,     2,     1,     1,     1,     5,
       5,     1,     1,     1,     1,     1,     1,     1,     6,     6,
       7,     7,    10,    10,     9,     9,     7,     7,     5,     5,
       6,     6,     7,     7,    10,    10,     6,     7,     6,     5,
       6,     4,     1,     2,     3,     5,     7,     6,     3,     1,
       3,     4,     6,     5,     1,     2,     4,     4,     5,     5,
       2,     3,     1,     3,     2,     1,     2,     3,     3,     3,
       4,     4,     4,     4,     4,     1,     1,     1,     1,     1,
       1,     0,     2,     1,     2,     2,     4,     4,     3,     3,
       1,     1,     2,     2,     3,     3,     4,     1,     1,     2
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,     0,   121,     0,     0,     0,   121,   121,     0,     0,
       0,   138,    36,    37,     0,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     4,     0,     0,   121,
     121,    38,    41,    43,   137,    44,    47,    57,    51,    48,
      46,    53,    52,    54,    55,    56,     0,   123,   130,   131,
       0,     3,   105,     0,     0,   121,   121,     0,   121,     0,
       0,   121,     0,   132,     0,   139,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    13,    21,
      17,    29,    15,    23,    19,    27,    25,    31,    33,    34,
       7,     8,     0,     0,    36,    42,    39,    45,   116,   117,
       1,   121,   121,   124,   125,   121,     0,   133,   121,   122,
     104,   106,   115,     0,   121,     0,   121,   119,   118,   120,
     121,   121,   121,     0,   121,   121,     0,     0,    90,   135,
     121,    12,    14,    22,    18,    30,    16,    24,    20,    28,
      26,    32,    35,     9,    10,    88,    84,    40,     0,     0,
     128,   129,     0,   134,     0,   121,   121,   121,   121,   121,
     121,     0,   121,     0,   121,     0,     0,     0,     0,   121,
       0,   121,     0,     0,   121,    82,    81,     0,   126,   127,
       0,     0,   136,   121,   121,    85,     0,     0,     0,   108,
     109,   107,     0,    94,   121,     0,   121,   121,     0,     5,
       0,   121,     0,    68,    69,   121,   121,   121,   121,     0,
       0,     0,     0,    49,    50,     0,    83,    79,     0,     0,
      87,   110,   111,   112,   113,   114,    78,   100,    95,     0,
      76,   102,     0,     0,     0,     0,    58,     6,   121,     0,
      59,     0,     0,     0,     0,    70,     0,   121,    71,    80,
      86,   121,   121,   121,   121,   101,    77,     0,     0,   121,
      60,    61,     0,   121,   121,    66,    67,    72,    73,     0,
      91,     0,     0,     0,   121,   103,    96,    97,   121,   121,
       0,     0,   121,   121,   121,    93,    98,    99,     0,     0,
      64,    65,     0,     0,    92,    62,    63,    74,    75,     0,
       0,     0
};

static const short yydefgoto[] =
{
     299,   200,    31,    32,    97,    33,    34,    35,    36,    37,
      38,    39,    40,   176,    41,    42,    43,    44,    45,   186,
     192,   193,   194,   233,    52,    53,   110,   111,   100,   121,
      54,    46,   150,   112,    49,    50
};

static const short yypact[] =
{
     273,   -24,-32768,    -2,    11,     6,-32768,-32768,    12,     9,
     402,    41,    10,-32768,   552,-32768,    46,    52,    -5,    58,
      64,    68,   102,   117,   135,-32768,-32768,   146,   149,-32768,
  -32768,-32768,-32768,   169,-32768,   202,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,   -33,    42,-32768,    91,
     445,-32768,-32768,   142,   316,-32768,   133,    72,   136,   172,
     174,    97,   171,    91,   531,-32768,   139,   173,   182,    99,
     188,   138,   189,   190,   191,   194,   195,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,   145,   168,-32768,-32768,-32768,   202,-32768,-32768,
  -32768,-32768,-32768,   359,   359,-32768,   531,    91,-32768,-32768,
  -32768,   249,-32768,   -12,-32768,    13,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,    57,-32768,-32768,   170,    39,-32768,    91,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   316,   316,
      28,    28,   488,    91,    74,-32768,-32768,-32768,-32768,-32768,
  -32768,    90,-32768,   220,-32768,   183,   178,   110,   113,-32768,
     198,-32768,   209,   215,-32768,   202,-32768,    39,-32768,-32768,
     359,   359,    91,-32768,-32768,-32768,   224,   316,   316,   316,
     316,   316,   230,   196,-32768,    16,-32768,-32768,   231,-32768,
     258,-32768,   197,-32768,-32768,-32768,-32768,-32768,-32768,   236,
     316,   258,   201,-32768,-32768,    39,   202,-32768,   247,   255,
  -32768,-32768,-32768,    66,    66,    66,-32768,-32768,   223,    17,
  -32768,-32768,   237,    92,   251,   211,-32768,-32768,-32768,   121,
  -32768,   252,   218,   256,   219,-32768,   249,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,   115,   253,-32768,
  -32768,-32768,   122,-32768,-32768,-32768,-32768,-32768,-32768,   125,
     106,   316,   316,   316,-32768,-32768,-32768,   316,-32768,-32768,
     260,   245,-32768,-32768,-32768,-32768,-32768,   316,   272,   259,
  -32768,-32768,   289,   264,-32768,-32768,-32768,-32768,-32768,   288,
     322,-32768
};

static const short yypgoto[] =
{
  -32768,   160,   -32,   283,   156,-32768,-32768,  -122,-32768,-32768,
  -32768,-32768,-32768,  -167,-32768,-32768,-32768,-32768,-32768,    65,
  -32768,   140,-32768,   104,  -162,    -6,-32768,  -173,-32768,  -157,
     -27,-32768,     4,     2,     3,   329
};


#define	YYLAST		600


static const short yytable[] =
{
      59,    60,    48,    96,    47,   175,   201,   209,   160,   212,
     217,    98,    99,    63,   221,   222,   223,   224,   225,    79,
      51,    80,    55,    92,    93,   230,   256,   162,   113,   115,
      58,   123,   109,   163,   127,    56,    61,   246,    62,    57,
     231,   231,     2,   238,    81,   243,   244,     3,   249,     4,
       5,     6,     7,   107,   247,   175,     9,   109,   101,   102,
     109,   109,    66,   164,    65,   147,    15,   129,   232,   232,
      77,   169,   101,   102,   148,   149,    78,   170,   152,   183,
     184,   185,    82,   109,   103,   104,   116,   161,    83,    29,
      84,    30,    86,   175,   167,   168,   155,   156,   223,   224,
     225,   109,   154,   177,   196,    48,    48,   171,   151,   153,
     165,   284,   184,    85,   166,   117,   118,   119,   172,   173,
     292,   293,   120,   133,   205,   134,    87,   207,   187,   188,
     189,   190,   191,   195,   109,   263,   278,   105,   258,   282,
     197,    88,   210,    96,   210,   259,   108,   215,   135,   126,
      48,    48,   178,   179,   109,   182,   198,   109,   202,    89,
     206,   258,   137,   208,   138,   109,   109,   229,   274,   109,
      90,   264,   279,    91,   239,   283,   114,   218,   219,   122,
     210,   210,    48,    48,   147,   151,   124,   139,   125,   128,
     234,   235,   130,    94,    13,    14,   145,   131,   203,   241,
     242,    16,    17,    18,    19,    20,   132,    21,    22,    23,
      24,   262,   136,   140,   141,   142,    27,    28,   143,   144,
     269,   146,   199,   174,   213,   271,   272,   273,    14,   204,
     214,   220,   277,   227,    16,    17,    18,    19,    20,   226,
      21,    22,    23,    24,   199,   270,   236,   287,   240,    27,
      28,   245,   248,   276,   250,   210,   210,   280,   281,   251,
     255,   231,   261,   117,   118,   119,   260,   265,   286,   266,
     268,   267,   288,   289,     1,   290,     2,   275,   294,   155,
     156,     3,   237,     4,     5,     6,     7,   295,   300,     8,
       9,   157,   158,   159,    10,    11,   291,    12,    13,    14,
      15,   117,   118,   119,   297,    16,    17,    18,    19,    20,
     296,    21,    22,    23,    24,   298,    95,    25,    26,     2,
      27,    28,   301,    29,     3,    30,     4,     5,     6,     7,
     211,   216,     8,     9,   228,   285,   257,    10,    11,    64,
      12,    13,    14,    15,     0,     0,     0,     0,    16,    17,
      18,    19,    20,     0,    21,    22,    23,    24,     0,     0,
     109,     0,     2,    27,    28,     0,    29,     3,    30,     4,
       5,     6,     7,     0,     0,     8,     9,     0,     0,     0,
      10,    11,     0,    12,    13,    14,    15,     0,     0,     0,
       0,    16,    17,    18,    19,    20,     0,    21,    22,    23,
      24,     0,     0,     0,     0,     2,    27,    28,     0,    29,
       3,    30,     4,     5,     6,     7,     0,     0,     8,     9,
       0,     0,     0,     0,    11,     0,    12,    13,    14,    15,
       0,     0,     0,     0,    16,    17,    18,    19,    20,     0,
      21,    22,    23,    24,     0,     0,     0,     0,     2,    27,
      28,     0,    29,     3,    30,     4,     5,     6,     7,     0,
       0,     8,     9,     0,     0,     0,   106,     0,     0,    12,
      13,    14,    15,     0,     0,     0,     0,    16,    17,    18,
      19,    20,     0,    21,    22,    23,    24,     0,     0,     0,
       0,     2,    27,    28,     0,    29,     3,    30,     4,     5,
       6,     7,     0,     0,     8,     9,     0,     0,     0,     0,
       0,     0,    12,    13,    14,    15,     0,     0,     0,     0,
      16,    17,    18,    19,    20,     0,    21,    22,    23,    24,
       0,     0,   109,     0,     2,    27,    28,     0,    29,     3,
      30,     4,     5,     6,     7,     0,     0,     8,     9,     0,
       0,     0,     0,     0,     0,    12,    13,    14,    15,     0,
       0,     0,     0,    16,    17,    18,    19,    20,     0,    21,
      22,    23,    24,     0,     0,     0,     0,     0,    27,    28,
       0,    29,     0,    30,    67,    68,    69,    70,    71,     0,
      72,     0,    73,    74,     0,     0,     0,     0,     0,    75,
      76
};

static const short yycheck[] =
{
       6,     7,     0,    35,     0,   127,   163,   169,    20,   171,
     177,    44,    45,    10,   187,   188,   189,   190,   191,    24,
      44,    26,    24,    29,    30,     9,     9,    14,    55,    56,
      24,    58,    44,    20,    61,    24,    24,   210,    29,    28,
      24,    24,     3,   200,    49,   207,   208,     8,   215,    10,
      11,    12,    13,    50,   211,   177,    17,    44,    30,    31,
      44,    44,    52,    50,    23,    97,    27,    64,    52,    52,
      24,    14,    30,    31,   101,   102,    24,    20,   105,     5,
       6,     7,    24,    44,    42,    43,    14,   114,    24,    50,
      26,    52,    24,   215,   121,   122,    30,    31,   271,   272,
     273,    44,   108,   130,    14,   103,   104,    50,   104,   106,
     116,     5,     6,    49,   120,    43,    44,    45,   124,   125,
     282,   283,    50,    24,    14,    26,    24,    14,   155,   156,
     157,   158,   159,   160,    44,    14,    14,    46,    46,    14,
      50,    24,   169,   175,   171,    53,     4,   174,    49,    52,
     148,   149,   148,   149,    44,   152,   162,    44,   164,    24,
      50,    46,    24,    50,    26,    44,    44,   194,    53,    44,
      24,    50,    50,    24,   201,    50,    43,   183,   184,    43,
     207,   208,   180,   181,   216,   181,    14,    49,    14,    18,
     196,   197,    53,    24,    25,    26,    51,    24,    15,   205,
     206,    32,    33,    34,    35,    36,    24,    38,    39,    40,
      41,   238,    24,    24,    24,    24,    47,    48,    24,    24,
     247,    53,    24,    53,    15,   252,   253,   254,    26,    51,
      15,     7,   259,    37,    32,    33,    34,    35,    36,     9,
      38,    39,    40,    41,    24,   251,    15,   274,    51,    47,
      48,    15,    51,   259,     7,   282,   283,   263,   264,     4,
      37,    24,    51,    43,    44,    45,    15,    15,   274,    51,
      51,    15,   278,   279,     1,    15,     3,    24,   284,    30,
      31,     8,    24,    10,    11,    12,    13,    15,     0,    16,
      17,    42,    43,    44,    21,    22,    51,    24,    25,    26,
      27,    43,    44,    45,    15,    32,    33,    34,    35,    36,
      51,    38,    39,    40,    41,    51,    33,    44,    45,     3,
      47,    48,     0,    50,     8,    52,    10,    11,    12,    13,
     170,   175,    16,    17,   194,   270,   232,    21,    22,    10,
      24,    25,    26,    27,    -1,    -1,    -1,    -1,    32,    33,
      34,    35,    36,    -1,    38,    39,    40,    41,    -1,    -1,
      44,    -1,     3,    47,    48,    -1,    50,     8,    52,    10,
      11,    12,    13,    -1,    -1,    16,    17,    -1,    -1,    -1,
      21,    22,    -1,    24,    25,    26,    27,    -1,    -1,    -1,
      -1,    32,    33,    34,    35,    36,    -1,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,     3,    47,    48,    -1,    50,
       8,    52,    10,    11,    12,    13,    -1,    -1,    16,    17,
      -1,    -1,    -1,    -1,    22,    -1,    24,    25,    26,    27,
      -1,    -1,    -1,    -1,    32,    33,    34,    35,    36,    -1,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,     3,    47,
      48,    -1,    50,     8,    52,    10,    11,    12,    13,    -1,
      -1,    16,    17,    -1,    -1,    -1,    21,    -1,    -1,    24,
      25,    26,    27,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    -1,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,     3,    47,    48,    -1,    50,     8,    52,    10,    11,
      12,    13,    -1,    -1,    16,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    25,    26,    27,    -1,    -1,    -1,    -1,
      32,    33,    34,    35,    36,    -1,    38,    39,    40,    41,
      -1,    -1,    44,    -1,     3,    47,    48,    -1,    50,     8,
      52,    10,    11,    12,    13,    -1,    -1,    16,    17,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    25,    26,    27,    -1,
      -1,    -1,    -1,    32,    33,    34,    35,    36,    -1,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    50,    -1,    52,    32,    33,    34,    35,    36,    -1,
      38,    -1,    40,    41,    -1,    -1,    -1,    -1,    -1,    47,
      48
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Relocate the TYPE STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Type, Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	yymemcpy ((char *) yyptr, (char *) (Stack),			\
		  yysize * (YYSIZE_T) sizeof (Type));			\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (Type) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#if ! defined (yyoverflow) && ! defined (yymemcpy)
# if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#  define yymemcpy __builtin_memcpy
# else				/* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
#  if defined (__STDC__) || defined (__cplusplus)
yymemcpy (char *yyto, const char *yyfrom, YYSIZE_T yycount)
#  else
yymemcpy (yyto, yyfrom, yycount)
     char *yyto;
     const char *yyfrom;
     YYSIZE_T yycount;
#  endif
{
  register const char *yyf = yyfrom;
  register char *yyt = yyto;
  register YYSIZE_T yyi = yycount;

  while (yyi-- != 0)
    *yyt++ = *yyf++;
}
# endif
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 319 "/usr/local/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (short, yyss);
	YYSTACK_RELOCATE (YYSTYPE, yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (YYLTYPE, yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 327 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = yyvsp[-1].command;
			  eof_encountered = 0;
			  /* discard_parser_constructs (0); */
			  YYACCEPT;
			}
    break;
case 2:
#line 336 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			}
    break;
case 3:
#line 343 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  /* discard_parser_constructs (1); */
			  if (interactive)
			    {
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			}
    break;
case 4:
#line 358 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Case of EOF seen by itself.  Do ignoreeof or
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			}
    break;
case 5:
#line 368 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); }
    break;
case 6:
#line 370 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-1].word_list); }
    break;
case 7:
#line 374 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_direction, redir);
			}
    break;
case 8:
#line 379 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_direction, redir);
			}
    break;
case 9:
#line 384 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_direction, redir);
			}
    break;
case 10:
#line 389 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_direction, redir);
			}
    break;
case 11:
#line 394 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_appending_to, redir);
			}
    break;
case 12:
#line 399 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_appending_to, redir);
			}
    break;
case 13:
#line 404 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
    break;
case 14:
#line 410 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
    break;
case 15:
#line 416 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_reading_string, redir);
			}
    break;
case 16:
#line 421 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_reading_string, redir);
			}
    break;
case 17:
#line 426 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (0, r_duplicating_input, redir);
			}
    break;
case 18:
#line 431 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input, redir);
			}
    break;
case 19:
#line 436 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (1, r_duplicating_output, redir);
			}
    break;
case 20:
#line 441 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output, redir);
			}
    break;
case 21:
#line 446 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_duplicating_input_word, redir);
			}
    break;
case 22:
#line 451 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input_word, redir);
			}
    break;
case 23:
#line 456 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_duplicating_output_word, redir);
			}
    break;
case 24:
#line 461 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output_word, redir);
			}
    break;
case 25:
#line 466 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (0, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
    break;
case 26:
#line 473 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
    break;
case 27:
#line 480 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (1, r_close_this, redir);
			}
    break;
case 28:
#line 485 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			}
    break;
case 29:
#line 490 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (0, r_close_this, redir);
			}
    break;
case 30:
#line 495 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			}
    break;
case 31:
#line 500 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_err_and_out, redir);
			}
    break;
case 32:
#line 505 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_output, redir);
			}
    break;
case 33:
#line 510 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_output, redir);
			}
    break;
case 34:
#line 515 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_force, redir);
			}
    break;
case 35:
#line 520 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_force, redir);
			}
    break;
case 36:
#line 527 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; }
    break;
case 37:
#line 529 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; }
    break;
case 38:
#line 531 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.element.redirect = yyvsp[0].redirect; yyval.element.word = 0; }
    break;
case 39:
#line 535 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.redirect = yyvsp[0].redirect;
			}
    break;
case 40:
#line 539 "/usr/homes/chet/src/bash/src/parse.y"
{
			  register REDIRECT *t;

			  for (t = yyvsp[-1].redirect; t->next; t = t->next)
			    ;
			  t->next = yyvsp[0].redirect;
			  yyval.redirect = yyvsp[-1].redirect;
			}
    break;
case 41:
#line 550 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, (COMMAND *)NULL); }
    break;
case 42:
#line 552 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, yyvsp[-1].command); }
    break;
case 43:
#line 556 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = clean_simple_command (yyvsp[0].command); }
    break;
case 44:
#line 558 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 45:
#line 560 "/usr/homes/chet/src/bash/src/parse.y"
{
			  COMMAND *tc;

			  tc = yyvsp[-1].command;
			  if (tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = yyvsp[0].redirect;
			    }
			  else
			    tc->redirects = yyvsp[0].redirect;
			  yyval.command = yyvsp[-1].command;
			}
    break;
case 46:
#line 576 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 47:
#line 580 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 48:
#line 582 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 49:
#line 584 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_while_command (yyvsp[-3].command, yyvsp[-1].command); }
    break;
case 50:
#line 586 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_until_command (yyvsp[-3].command, yyvsp[-1].command); }
    break;
case 51:
#line 588 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 52:
#line 590 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 53:
#line 592 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 54:
#line 594 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 55:
#line 596 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 56:
#line 598 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 57:
#line 600 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 58:
#line 604 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); }
    break;
case 59:
#line 606 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command); }
    break;
case 60:
#line 608 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); }
    break;
case 61:
#line 610 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); }
    break;
case 62:
#line 612 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); }
    break;
case 63:
#line 614 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); }
    break;
case 64:
#line 616 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-7].word, (WORD_LIST *)NULL, yyvsp[-1].command); }
    break;
case 65:
#line 618 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-7].word, (WORD_LIST *)NULL, yyvsp[-1].command); }
    break;
case 66:
#line 622 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-5].word_list, yyvsp[-1].command, arith_for_lineno); }
    break;
case 67:
#line 624 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-5].word_list, yyvsp[-1].command, arith_for_lineno); }
    break;
case 68:
#line 626 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-3].word_list, yyvsp[-1].command, arith_for_lineno); }
    break;
case 69:
#line 628 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-3].word_list, yyvsp[-1].command, arith_for_lineno); }
    break;
case 70:
#line 632 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
    break;
case 71:
#line 636 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
    break;
case 72:
#line 640 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
    break;
case 73:
#line 644 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
    break;
case 74:
#line 648 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command);
			}
    break;
case 75:
#line 652 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command);
			}
    break;
case 76:
#line 658 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, (PATTERN_LIST *)NULL); }
    break;
case 77:
#line 660 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_case_command (yyvsp[-5].word, yyvsp[-2].pattern); }
    break;
case 78:
#line 662 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, yyvsp[-1].pattern); }
    break;
case 79:
#line 666 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command, function_dstart, function_bstart); }
    break;
case 80:
#line 669 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command, function_dstart, function_bstart); }
    break;
case 81:
#line 672 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_function_def (yyvsp[-2].word, yyvsp[0].command, function_dstart, function_bstart); }
    break;
case 82:
#line 677 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 83:
#line 679 "/usr/homes/chet/src/bash/src/parse.y"
{
			  COMMAND *tc;

			  tc = yyvsp[-1].command;
			  /* According to Posix.2 3.9.5, redirections
			     specified after the body of a function should
			     be attached to the function and performed when
			     the function is executed, not as part of the
			     function definition command. */
			  /* XXX - I don't think it matters, but we might
			     want to change this in the future to avoid
			     problems differentiating between a function
			     definition with a redirection and a function
			     definition containing a single command with a
			     redirection.  The two are semantically equivalent,
			     though -- the only difference is in how the
			     command printing code displays the redirections. */
			  if (tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = yyvsp[0].redirect;
			    }
			  else
			    tc->redirects = yyvsp[0].redirect;
			  yyval.command = yyvsp[-1].command;
			}
    break;
case 84:
#line 710 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_subshell_command (yyvsp[-1].command);
			  yyval.command->flags |= CMD_WANT_SUBSHELL;
			}
    break;
case 85:
#line 717 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, (COMMAND *)NULL); }
    break;
case 86:
#line 719 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-5].command, yyvsp[-3].command, yyvsp[-1].command); }
    break;
case 87:
#line 721 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[-1].command); }
    break;
case 88:
#line 726 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_group_command (yyvsp[-1].command); }
    break;
case 89:
#line 730 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_command (yyvsp[0].word_list); }
    break;
case 90:
#line 734 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[-1].command; }
    break;
case 91:
#line 738 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-2].command, yyvsp[0].command, (COMMAND *)NULL); }
    break;
case 92:
#line 740 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[0].command); }
    break;
case 93:
#line 742 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, yyvsp[0].command); }
    break;
case 95:
#line 747 "/usr/homes/chet/src/bash/src/parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; }
    break;
case 96:
#line 751 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); }
    break;
case 97:
#line 753 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); }
    break;
case 98:
#line 755 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); }
    break;
case 99:
#line 757 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); }
    break;
case 101:
#line 762 "/usr/homes/chet/src/bash/src/parse.y"
{ yyvsp[-1].pattern->next = yyvsp[-2].pattern; yyval.pattern = yyvsp[-1].pattern; }
    break;
case 102:
#line 766 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); }
    break;
case 103:
#line 768 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-2].word_list); }
    break;
case 104:
#line 777 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			 }
    break;
case 106:
#line 786 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[0].command;
			}
    break;
case 108:
#line 793 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, (COMMAND *)NULL, '&');
			}
    break;
case 110:
#line 804 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); }
    break;
case 111:
#line 806 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); }
    break;
case 112:
#line 808 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-3].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-3].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '&');
			}
    break;
case 113:
#line 815 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); }
    break;
case 114:
#line 817 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); }
    break;
case 115:
#line 819 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 123:
#line 842 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			}
    break;
case 124:
#line 848 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-1].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents ();
			}
    break;
case 125:
#line 857 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[-1].command;
			  if (need_here_doc)
			    gather_here_documents ();
			}
    break;
case 126:
#line 865 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); }
    break;
case 127:
#line 867 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); }
    break;
case 128:
#line 869 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, '&');
			}
    break;
case 129:
#line 876 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, ';'); }
    break;
case 130:
#line 879 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 131:
#line 883 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 132:
#line 885 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			}
    break;
case 133:
#line 890 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-1].number;
			  yyval.command = yyvsp[0].command;
			}
    break;
case 134:
#line 895 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-2].number|CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			}
    break;
case 135:
#line 900 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-1].number|CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			}
    break;
case 136:
#line 908 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '|'); }
    break;
case 137:
#line 910 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
    break;
case 138:
#line 914 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.number = CMD_TIME_PIPELINE; }
    break;
case 139:
#line 916 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.number = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
    break;
}

#line 705 "/usr/local/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 918 "/usr/homes/chet/src/bash/src/parse.y"


/* Possible states for the parser that require it to do special things. */
#define PST_CASEPAT	0x001		/* in a case pattern list */
#define PST_ALEXPNEXT	0x002		/* expand next word for aliases */
#define PST_ALLOWOPNBRC	0x004		/* allow open brace for function def */
#define PST_NEEDCLOSBRC	0x008		/* need close brace */
#define PST_DBLPAREN	0x010		/* double-paren parsing */
#define PST_SUBSHELL	0x020		/* ( ... ) subshell */
#define PST_CMDSUBST	0x040		/* $( ... ) command substitution */
#define PST_CASESTMT	0x080		/* parsing a case statement */
#define PST_CONDCMD	0x100		/* parsing a [[...]] command */
#define PST_CONDEXPR	0x200		/* parsing the guts of [[...]] */
#define PST_ARITHFOR	0x400		/* parsing an arithmetic for command */

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_INITIAL_SIZE 496
#define TOKEN_DEFAULT_GROW_SIZE 512

/* The token currently being read. */
static int current_token;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
static int last_read_token;

/* The token read prior to last_read_token. */
static int token_before_that;

/* The token read prior to token_before_that. */
static int two_tokens_ago;

/* If non-zero, it is the token that we want read_token to return
   regardless of what text is (or isn't) present to be read.  This
   is reset by read_token.  If token_to_read == WORD or
   ASSIGNMENT_WORD, yylval.word should be set to word_desc_to_read. */
static int token_to_read;
static WORD_DESC *word_desc_to_read;

/* The current parser state. */
static int parser_state;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

#ifdef DEBUG
static void
debug_parser (i)
     int i;
{
#if YYDEBUG != 0
  yydebug = i;
#endif
}
#endif

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget, type, location) makes the function GET the
   installed function for getting the next character, makes UNGET the
   installed function for un-getting a character, sets the type of stream
   (either string or file) from TYPE, and makes LOCATION point to where
   the input is coming from. */

/* Unconditionally returns end-of-file. */
int
return_EOF ()
{
  return (EOF);
}

/* Variable containing the current get and unget functions.
   See ./input.h for a clearer description. */
BASH_INPUT bash_input;

/* Set all of the fields in BASH_INPUT to NULL.  Free bash_input.name if it
   is non-null, avoiding a memory leak. */
void
initialize_bash_input ()
{
  bash_input.type = st_none;
  FREE (bash_input.name);
  bash_input.name = (char *)NULL;
  bash_input.location.file = (FILE *)NULL;
  bash_input.location.string = (char *)NULL;
  bash_input.getter = (sh_cget_func_t *)NULL;
  bash_input.ungetter = (sh_cunget_func_t *)NULL;
}

/* Set the contents of the current bash input stream from
   GET, UNGET, TYPE, NAME, and LOCATION. */
void
init_yy_io (get, unget, type, name, location)
     sh_cget_func_t *get;
     sh_cunget_func_t *unget;
     enum stream_type type;
     const char *name;
     INPUT_STREAM location;
{
  bash_input.type = type;
  FREE (bash_input.name);
  bash_input.name = name ? savestring (name) : (char *)NULL;

  /* XXX */
#if defined (CRAY)
  memcpy((char *)&bash_input.location.string, (char *)&location.string, sizeof(location));
#else
  bash_input.location = location;
#endif
  bash_input.getter = get;
  bash_input.ungetter = unget;
}

char *
yy_input_name ()
{
  return (bash_input.name ? bash_input.name : "stdin");
}

/* Call this to get the next character of input. */
static int
yy_getc ()
{
  return (*(bash_input.getter)) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
static int
yy_ungetc (c)
     int c;
{
  return (*(bash_input.ungetter)) (c);
}

#if defined (BUFFERED_INPUT)
#ifdef INCLUDE_UNUSED
int
input_file_descriptor ()
{
  switch (bash_input.type)
    {
    case st_stream:
      return (fileno (bash_input.location.file));
    case st_bstream:
      return (bash_input.location.buffered_fd);
    case st_stdin:
    default:
      return (fileno (stdin));
    }
}
#endif
#endif /* BUFFERED_INPUT */

/* **************************************************************** */
/*								    */
/*		  Let input be read from readline ().		    */
/*								    */
/* **************************************************************** */

#if defined (READLINE)
char *current_readline_prompt = (char *)NULL;
char *current_readline_line = (char *)NULL;
int current_readline_line_index = 0;

static int
yy_readline_get ()
{
  SigHandler *old_sigint;
  int line_len;
  unsigned char c;

  if (!current_readline_line)
    {
      if (!bash_readline_initialized)
	initialize_readline ();

#if defined (JOB_CONTROL)
      if (job_control)
	give_terminal_to (shell_pgrp, 0);
#endif /* JOB_CONTROL */

      old_sigint = (SigHandler *)NULL;
      if (signal_is_ignored (SIGINT) == 0)
	{
	  old_sigint = (SigHandler *)set_signal_handler (SIGINT, sigint_sighandler);
	  interrupt_immediately++;
	}

      current_readline_line = readline (current_readline_prompt ?
      					  current_readline_prompt : "");

      if (signal_is_ignored (SIGINT) == 0 && old_sigint)
	{
	  interrupt_immediately--;
	  set_signal_handler (SIGINT, old_sigint);
	}

#if 0
      /* Reset the prompt to the decoded value of prompt_string_pointer. */
      reset_readline_prompt ();
#endif

      if (current_readline_line == 0)
	return (EOF);

      current_readline_line_index = 0;
      line_len = strlen (current_readline_line);

      current_readline_line = (char *)xrealloc (current_readline_line, 2 + line_len);
      current_readline_line[line_len++] = '\n';
      current_readline_line[line_len] = '\0';
    }

  if (current_readline_line[current_readline_line_index] == 0)
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      c = current_readline_line[current_readline_line_index++];
      return (c);
    }
}

static int
yy_readline_unget (c)
     int c;
{
  if (current_readline_line_index && current_readline_line)
    current_readline_line[--current_readline_line_index] = c;
  return (c);
}

void
with_input_from_stdin ()
{
  INPUT_STREAM location;

  if (bash_input.type != st_stdin && stream_on_stack (st_stdin) == 0)
    {
      location.string = current_readline_line;
      init_yy_io (yy_readline_get, yy_readline_unget,
		  st_stdin, "readline stdin", location);
    }
}

#else  /* !READLINE */

void
with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}
#endif	/* !READLINE */

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

static int
yy_string_get ()
{
  register char *string;
  register unsigned char c;

  string = bash_input.location.string;

  /* If the string doesn't exist, or is empty, EOF found. */
  if (string && *string)
    {
      c = *string++;
      bash_input.location.string = string;
      return (c);
    }
  else
    return (EOF);
}

static int
yy_string_unget (c)
     int c;
{
  *(--bash_input.location.string) = c;
  return (c);
}

void
with_input_from_string (string, name)
     char *string;
     const char *name;
{
  INPUT_STREAM location;

  location.string = string;
  init_yy_io (yy_string_get, yy_string_unget, st_string, name, location);
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

/* These two functions used to test the value of the HAVE_RESTARTABLE_SYSCALLS
   define, and just use getc/ungetc if it was defined, but since bash
   installs its signal handlers without the SA_RESTART flag, some signals
   (like SIGCHLD, SIGWINCH, etc.) received during a read(2) will not cause
   the read to be restarted.  We need to restart it ourselves. */

static int
yy_stream_get ()
{
  int result;

  result = EOF;
  if (bash_input.location.file)
    result = getc_with_restart (bash_input.location.file);

  return (result);
}

static int
yy_stream_unget (c)
     int c;
{
  return (ungetc_with_restart (c, bash_input.location.file));
}

void
with_input_from_stream (stream, name)
     FILE *stream;
     const char *name;
{
  INPUT_STREAM location;

  location.file = stream;
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, name, location);
}

typedef struct stream_saver {
  struct stream_saver *next;
  BASH_INPUT bash_input;
  int line;
#if defined (BUFFERED_INPUT)
  BUFFERED_STREAM *bstream;
#endif /* BUFFERED_INPUT */
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

#if defined (COND_COMMAND)
static int cond_lineno;
static int cond_token;
#endif

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

void
push_stream (reset_lineno)
     int reset_lineno;
{
  STREAM_SAVER *saver = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));

  xbcopy ((char *)&bash_input, (char *)&(saver->bash_input), sizeof (BASH_INPUT));

#if defined (BUFFERED_INPUT)
  saver->bstream = (BUFFERED_STREAM *)NULL;
  /* If we have a buffered stream, clear out buffers[fd]. */
  if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
    saver->bstream = set_buffered_stream (bash_input.location.buffered_fd,
    					  (BUFFERED_STREAM *)NULL);
#endif /* BUFFERED_INPUT */

  saver->line = line_number;
  bash_input.name = (char *)NULL;
  saver->next = stream_list;
  stream_list = saver;
  EOF_Reached = 0;
  if (reset_lineno)
    line_number = 0;
}

void
pop_stream ()
{
  if (!stream_list)
    EOF_Reached = 1;
  else
    {
      STREAM_SAVER *saver = stream_list;

      EOF_Reached = 0;
      stream_list = stream_list->next;

      init_yy_io (saver->bash_input.getter,
		  saver->bash_input.ungetter,
		  saver->bash_input.type,
		  saver->bash_input.name,
		  saver->bash_input.location);

#if defined (BUFFERED_INPUT)
      /* If we have a buffered stream, restore buffers[fd]. */
      /* If the input file descriptor was changed while this was on the
	 save stack, update the buffered fd to the new file descriptor and
	 re-establish the buffer <-> bash_input fd correspondence. */
      if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
	{
	  if (bash_input_fd_changed)
	    {
	      bash_input_fd_changed = 0;
	      if (default_buffered_input >= 0)
		{
		  bash_input.location.buffered_fd = default_buffered_input;
		  saver->bstream->b_fd = default_buffered_input;
		  SET_CLOSE_ON_EXEC (default_buffered_input);
		}
	    }
	  /* XXX could free buffered stream returned as result here. */
	  set_buffered_stream (bash_input.location.buffered_fd, saver->bstream);
	}
#endif /* BUFFERED_INPUT */

      line_number = saver->line;

      FREE (saver->bash_input.name);
      free (saver);
    }
}

/* Return 1 if a stream of type TYPE is saved on the stack. */
int
stream_on_stack (type)
     enum stream_type type;
{
  register STREAM_SAVER *s;

  for (s = stream_list; s; s = s->next)
    if (s->bash_input.type == type)
      return 1;
  return 0;
}

/* Save the current token state and return it in a malloced array. */
int *
save_token_state ()
{
  int *ret;

  ret = (int *)xmalloc (3 * sizeof (int));
  ret[0] = last_read_token;
  ret[1] = token_before_that;
  ret[2] = two_tokens_ago;
  return ret;
}

void
restore_token_state (ts)
     int *ts;
{
  if (ts == 0)
    return;
  last_read_token = ts[0];
  token_before_that = ts[1];
  two_tokens_ago = ts[2];
}

/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list' is:
 *
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)

#if !defined (ALIAS)
typedef void *alias_t;
#endif

#define END_OF_ALIAS 0

/*
 * Pseudo-global variables used in implementing token-wise alias expansion.
 */

/*
 * Pushing and popping strings.  This works together with shell_getc to
 * implement alias expansion on a per-token basis.
 */

typedef struct string_saver {
  struct string_saver *next;
  int expand_alias;  /* Value to set expand_alias to when string is popped. */
  char *saved_line;
#if defined (ALIAS)
  alias_t *expander;   /* alias that caused this line to be pushed. */
#endif
  int saved_line_size, saved_line_index, saved_line_terminator;
} STRING_SAVER;

STRING_SAVER *pushed_string_list = (STRING_SAVER *)NULL;

/*
 * Push the current shell_input_line onto a stack of such lines and make S
 * the current input.  Used when expanding aliases.  EXPAND is used to set
 * the value of expand_next_token when the string is popped, so that the
 * word after the alias in the original line is handled correctly when the
 * alias expands to multiple words.  TOKEN is the token that was expanded
 * into S; it is saved and used to prevent infinite recursive expansion.
 */
static void
push_string (s, expand, ap)
     char *s;
     int expand;
     alias_t *ap;
{
  STRING_SAVER *temp = (STRING_SAVER *)xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
#if defined (ALIAS)
  temp->expander = ap;
#endif
  temp->next = pushed_string_list;
  pushed_string_list = temp;

#if defined (ALIAS)
  if (ap)
    ap->flags |= AL_BEINGEXPANDED;
#endif

  shell_input_line = s;
  shell_input_line_size = strlen (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
  parser_state &= ~PST_ALEXPNEXT;

  set_line_mbstate ();
}

/*
 * Make the top of the pushed_string stack be the current shell input.
 * Only called when there is something on the stack.  Called from shell_getc
 * when it thinks it has consumed the string generated by an alias expansion
 * and needs to return to the original input line.
 */
static void
pop_string ()
{
  STRING_SAVER *t;

  FREE (shell_input_line);
  shell_input_line = pushed_string_list->saved_line;
  shell_input_line_index = pushed_string_list->saved_line_index;
  shell_input_line_size = pushed_string_list->saved_line_size;
  shell_input_line_terminator = pushed_string_list->saved_line_terminator;

  if (pushed_string_list->expand_alias)
    parser_state |= PST_ALEXPNEXT;
  else
    parser_state &= ~PST_ALEXPNEXT;

  t = pushed_string_list;
  pushed_string_list = pushed_string_list->next;

#if defined (ALIAS)
  if (t->expander)
    t->expander->flags &= ~AL_BEINGEXPANDED;
#endif

  free ((char *)t);

  set_line_mbstate ();
}

static void
free_string_list ()
{
  register STRING_SAVER *t, *t1;

  for (t = pushed_string_list; t; )
    {
      t1 = t->next;
      FREE (t->saved_line);
#if defined (ALIAS)
      if (t->expander)
	t->expander->flags &= ~AL_BEINGEXPANDED;
#endif
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

#endif /* ALIAS || DPAREN_ARITHMETIC */

/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL.  If REMOVE_QUOTED_NEWLINE
   is non-zero, we remove unquoted \<newline> pairs.  This is used by
   read_secondary_line to read here documents. */
static char *
read_a_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  static char *line_buffer = (char *)NULL;
  static int buffer_size = 0;
  int indx = 0, c, peekc, pass_next;

#if defined (READLINE)
  if (interactive && bash_input.type != st_string && no_line_editing)
#else
  if (interactive && bash_input.type != st_string)
#endif
    print_prompt ();

  pass_next = 0;
  while (1)
    {
      c = yy_getc ();

      /* Allow immediate exit if interrupted during input. */
      QUIT;

      /* Ignore null bytes in input. */
      if (c == 0)
	{
#if 0
	  internal_warning ("read_a_line: ignored null byte in input");
#endif
	  continue;
	}

      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  if (interactive && bash_input.type == st_stream)
	    clearerr (stdin);
	  if (indx == 0)
	    return ((char *)NULL);
	  c = '\n';
	}

      /* `+2' in case the final character in the buffer is a newline. */
      RESIZE_MALLOCED_BUFFER (line_buffer, indx, 2, buffer_size, 128);

      /* IF REMOVE_QUOTED_NEWLINES is non-zero, we are reading a
	 here document with an unquoted delimiter.  In this case,
	 the line will be expanded as if it were in double quotes.
	 We allow a backslash to escape the next character, but we
	 need to treat the backslash specially only if a backslash
	 quoting a backslash-newline pair appears in the line. */
      if (pass_next)
	{
	  line_buffer[indx++] = c;
	  pass_next = 0;
	}
      else if (c == '\\' && remove_quoted_newline)
	{
	  peekc = yy_getc ();
	  if (peekc == '\n')
	    continue;	/* Make the unquoted \<newline> pair disappear. */
	  else
	    {
	      yy_ungetc (peekc);
	      pass_next = 1;
	      line_buffer[indx++] = c;		/* Preserve the backslash. */
	    }
	}
      else
	line_buffer[indx++] = c;

      if (c == '\n')
	{
	  line_buffer[indx] = '\0';
	  return (line_buffer);
	}
    }
}

/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt.  This is used to read the lines of a here
   document.  REMOVE_QUOTED_NEWLINE is non-zero if we should remove
   newlines quoted with backslashes while reading the line.  It is
   non-zero unless the delimiter of the here document was quoted. */
char *
read_secondary_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  prompt_string_pointer = &ps2_prompt;
  prompt_again ();
  return (read_a_line (remove_quoted_newline));
}

/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command. */
STRING_INT_ALIST word_token_alist[] = {
  { "if", IF },
  { "then", THEN },
  { "else", ELSE },
  { "elif", ELIF },
  { "fi", FI },
  { "case", CASE },
  { "esac", ESAC },
  { "for", FOR },
#if defined (SELECT_COMMAND)
  { "select", SELECT },
#endif
  { "while", WHILE },
  { "until", UNTIL },
  { "do", DO },
  { "done", DONE },
  { "in", IN },
  { "function", FUNCTION },
#if defined (COMMAND_TIMING)
  { "time", TIME },
#endif
  { "{", '{' },
  { "}", '}' },
  { "!", BANG },
#if defined (COND_COMMAND)
  { "[[", COND_START },
  { "]]", COND_END },
#endif
  { (char *)NULL, 0}
};

/* other tokens that can be returned by read_token() */
STRING_INT_ALIST other_token_alist[] = {
  /* Multiple-character tokens with special values */
  { "-p", TIMEOPT },
  { "&&", AND_AND },
  { "||", OR_OR },
  { ">>", GREATER_GREATER },
  { "<<", LESS_LESS },
  { "<&", LESS_AND },
  { ">&", GREATER_AND },
  { ";;", SEMI_SEMI },
  { "<<-", LESS_LESS_MINUS },
  { "<<<", LESS_LESS_LESS },
  { "&>", AND_GREATER },
  { "<>", LESS_GREATER },
  { ">|", GREATER_BAR },
  { "EOF", yacc_EOF },
  /* Tokens whose value is the character itself */
  { ">", '>' },
  { "<", '<' },
  { "-", '-' },
  { "{", '{' },
  { "}", '}' },
  { ";", ';' },
  { "(", '(' },
  { ")", ')' },
  { "|", '|' },
  { "&", '&' },
  { "newline", '\n' },
  { (char *)NULL, 0}
};

/* others not listed here:
	WORD			look at yylval.word
	ASSIGNMENT_WORD		look at yylval.word
	NUMBER			look at yylval.number
	ARITH_CMD		look at yylval.word_list
	ARITH_FOR_EXPRS		look at yylval.word_list
	COND_CMD		look at yylval.command
*/

/* These are used by read_token_word, but appear up here so that shell_getc
   can use them to decide when to add otherwise blank lines to the history. */

/* The primary delimiter stack. */
struct dstack dstack = {  (char *)NULL, 0, 0 };

/* A temporary delimiter stack to be used when decoding prompt strings.
   This is needed because command substitutions in prompt strings (e.g., PS2)
   can screw up the parser's quoting state. */
static struct dstack temp_dstack = { (char *)NULL, 0, 0 };

/* Macro for accessing the top delimiter on the stack.  Returns the
   delimiter or zero if none. */
#define current_delimiter(ds) \
  (ds.delimiter_depth ? ds.delimiters[ds.delimiter_depth - 1] : 0)

#define push_delimiter(ds, character) \
  do \
    { \
      if (ds.delimiter_depth + 2 > ds.delimiter_space) \
	ds.delimiters = (char *)xrealloc \
	  (ds.delimiters, (ds.delimiter_space += 10) * sizeof (char)); \
      ds.delimiters[ds.delimiter_depth] = character; \
      ds.delimiter_depth++; \
    } \
  while (0)

#define pop_delimiter(ds)	ds.delimiter_depth--

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line.  This is called by read_token when the shell is
   processing normal command input. */

/* This implements one-character lookahead/lookbehind across physical input
   lines, to avoid something being lost because it's pushed back with
   shell_ungetc when we're at the start of a line. */
static int eol_ungetc_lookahead = 0;

static int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  register int i;
  int c;
  unsigned char uc;
  static int mustpop = 0;

  QUIT;

  if (eol_ungetc_lookahead)
    {
      c = eol_ungetc_lookahead;
      eol_ungetc_lookahead = 0;
      return (c);
    }

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If shell_input_line[shell_input_line_index] == 0, but there is
     something on the pushed list of strings, then we don't want to go
     off and get another line.  We let the code down below handle it. */

  if (!shell_input_line || ((!shell_input_line[shell_input_line_index]) &&
			    (pushed_string_list == (STRING_SAVER *)NULL)))
#else /* !ALIAS && !DPAREN_ARITHMETIC */
  if (!shell_input_line || !shell_input_line[shell_input_line_index])
#endif /* !ALIAS && !DPAREN_ARITHMETIC */
    {
      line_number++;

    restart_read:

      /* Allow immediate exit if interrupted during input. */
      QUIT;

      i = 0;
      shell_input_line_terminator = 0;

#if defined (JOB_CONTROL)
      /* This can cause a problem when reading a command as the result
	 of a trap, when the trap is called from flush_child.  This call
	 had better not cause jobs to disappear from the job table in
	 that case, or we will have big trouble. */
      notify_and_cleanup ();
#else /* !JOB_CONTROL */
      cleanup_dead_jobs ();
#endif /* !JOB_CONTROL */

#if defined (READLINE)
      if (interactive && bash_input.type != st_string && no_line_editing)
#else
      if (interactive && bash_input.type != st_string)
#endif
	print_prompt ();

      if (bash_input.type == st_stream)
	clearerr (stdin);

      while (1)
	{
	  c = yy_getc ();

	  /* Allow immediate exit if interrupted during input. */
	  QUIT;

	  if (c == '\0')
	    {
#if 0
	      internal_warning ("shell_getc: ignored null byte in input");
#endif
	      continue;
	    }

	  RESIZE_MALLOCED_BUFFER (shell_input_line, i, 2, shell_input_line_size, 256);

	  if (c == EOF)
	    {
	      if (bash_input.type == st_stream)
		clearerr (stdin);

	      if (i == 0)
		shell_input_line_terminator = EOF;

	      shell_input_line[i] = '\0';
	      break;
	    }

	  shell_input_line[i++] = c;

	  if (c == '\n')
	    {
	      shell_input_line[--i] = '\0';
	      current_command_line_count++;
	      break;
	    }
	}

      shell_input_line_index = 0;
      shell_input_line_len = i;		/* == strlen (shell_input_line) */

      set_line_mbstate ();

#if defined (HISTORY)
      if (remember_on_history && shell_input_line && shell_input_line[0])
	{
	  char *expansions;
#  if defined (BANG_HISTORY)
	  int old_hist;

	  /* If the current delimiter is a single quote, we should not be
	     performing history expansion, even if we're on a different
	     line from the original single quote. */
	  old_hist = history_expansion_inhibited;
	  if (current_delimiter (dstack) == '\'')
	    history_expansion_inhibited = 1;
#  endif
	  expansions = pre_process_line (shell_input_line, 1, 1);
#  if defined (BANG_HISTORY)
	  history_expansion_inhibited = old_hist;
#  endif
	  if (expansions != shell_input_line)
	    {
	      free (shell_input_line);
	      shell_input_line = expansions;
	      shell_input_line_len = shell_input_line ?
					strlen (shell_input_line) : 0;
	      if (!shell_input_line_len)
		current_command_line_count--;

	      /* We have to force the xrealloc below because we don't know
		 the true allocated size of shell_input_line anymore. */
	      shell_input_line_size = shell_input_line_len;

	      set_line_mbstate ();
	    }
	}
      /* Try to do something intelligent with blank lines encountered while
	 entering multi-line commands.  XXX - this is grotesque */
      else if (remember_on_history && shell_input_line &&
	       shell_input_line[0] == '\0' &&
	       current_command_line_count > 1)
	{
	  if (current_delimiter (dstack))
	    /* We know shell_input_line[0] == 0 and we're reading some sort of
	       quoted string.  This means we've got a line consisting of only
	       a newline in a quoted string.  We want to make sure this line
	       gets added to the history. */
	    maybe_add_history (shell_input_line);
	  else
	    {
	      char *hdcs;
	      hdcs = history_delimiting_chars ();
	      if (hdcs && hdcs[0] == ';')
		maybe_add_history (shell_input_line);
	    }
	}

#endif /* HISTORY */

      if (shell_input_line)
	{
	  /* Lines that signify the end of the shell's input should not be
	     echoed. */
	  if (echo_input_at_read && (shell_input_line[0] ||
				     shell_input_line_terminator != EOF))
	    fprintf (stderr, "%s\n", shell_input_line);
	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_string_pointer = &current_prompt_string;
	  prompt_again ();
	  goto restart_read;
	}

      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  if (shell_input_line_len + 3 > shell_input_line_size)
	    shell_input_line = (char *)xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  shell_input_line[shell_input_line_len] = '\n';
	  shell_input_line[shell_input_line_len + 1] = '\0';

	  set_line_mbstate ();
	}
    }

  uc = shell_input_line[shell_input_line_index];

  if (uc)
    shell_input_line_index++;

  if MBTEST(uc == '\\' && remove_quoted_newline && shell_input_line[shell_input_line_index] == '\n')
    {
	prompt_again ();
	line_number++;
	goto restart_read;
    }

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If UC is NULL, we have reached the end of the current input string.  If
     pushed_string_list is non-empty, it's time to pop to the previous string
     because we have fully consumed the result of the last alias expansion.
     Do it transparently; just return the next character of the string popped
     to. */
  if (!uc && (pushed_string_list != (STRING_SAVER *)NULL))
    {
      if (mustpop)
	{
	  pop_string ();
	  uc = shell_input_line[shell_input_line_index];
	  if (uc)
	    shell_input_line_index++;
	  mustpop--;
	}
      else
	{
	  mustpop++;
	  uc = ' ';
	}
    }
#endif /* ALIAS || DPAREN_ARITHMETIC */

  if (!uc && shell_input_line_terminator == EOF)
    return ((shell_input_line_index != 0) ? '\n' : EOF);

  return (uc);
}

/* Put C back into the input for the shell.  This might need changes for
   HANDLE_MULTIBYTE around EOLs.  Since we (currently) never push back a
   character different than we read, shell_input_line_property doesn't need
   to change when manipulating shell_input_line.  The define for
   last_shell_getc_is_singlebyte should take care of it, though. */
static void
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
  else
    eol_ungetc_lookahead = c;
}

#ifdef INCLUDE_UNUSED
/* Back the input pointer up by one, effectively `ungetting' a character. */
static void
shell_ungetchar ()
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line_index--;
}
#endif

/* Discard input until CHARACTER is seen, then push that character back
   onto the input stream. */
static void
discard_until (character)
     int character;
{
  int c;

  while ((c = shell_getc (0)) != EOF && c != character)
    ;

  if (c != EOF)
    shell_ungetc (c);
}

void
execute_prompt_command (command)
     char *command;
{
  sh_builtin_func_t *temp_last, *temp_this;
  char *last_lastarg;
  int temp_exit_value, temp_eof_encountered;

  temp_last = last_shell_builtin;
  temp_this = this_shell_builtin;
  temp_exit_value = last_command_exit_value;
  temp_eof_encountered = eof_encountered;
  last_lastarg = get_string_value ("_");
  if (last_lastarg)
    last_lastarg = savestring (last_lastarg);

  parse_and_execute (savestring (command), "PROMPT_COMMAND", SEVAL_NONINT|SEVAL_NOHIST);

  last_shell_builtin = temp_last;
  this_shell_builtin = temp_this;
  last_command_exit_value = temp_exit_value;
  eof_encountered = temp_eof_encountered;

  bind_variable ("_", last_lastarg);
  FREE (last_lastarg);

  if (token_to_read == '\n')	/* reset_parser was called */
    token_to_read = 0;
}

/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
static char *token = (char *)NULL;

/* Current size of the token buffer. */
static int token_buffer_size;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */
static int
yylex ()
{
  if (interactive && (current_token == 0 || current_token == '\n'))
    {
      /* Before we print a prompt, we might have to check mailboxes.
	 We do this only if it is time to do so. Notice that only here
	 is the mail alarm reset; nothing takes place in check_mail ()
	 except the checking of mail.  Please don't change this. */
      if (prompt_is_ps1 && time_to_check_mail ())
	{
	  check_mail ();
	  reset_mail_timer ();
	}

      /* Avoid printing a prompt if we're not going to read anything, e.g.
	 after resetting the parser with read_token (RESET). */
      if (token_to_read == 0 && interactive)
	prompt_again ();
    }

  two_tokens_ago = token_before_that;
  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int esacs_needed_count;

void
gather_here_documents ()
{
  int r = 0;
  while (need_here_doc)
    {
      make_here_document (redir_stack[r++]);
      need_here_doc--;
    }
}

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_count;

#define command_token_position(token) \
  (((token) == ASSIGNMENT_WORD) || \
   ((token) != SEMI_SEMI && reserved_word_acceptable(token)))

#define assignment_acceptable(token) command_token_position(token) && \
					((parser_state & PST_CASEPAT) == 0)

/* Check to see if TOKEN is a reserved word and return the token
   value if it is. */
#define CHECK_FOR_RESERVED_WORD(tok) \
  do { \
    if (!dollar_present && !quoted && \
	reserved_word_acceptable (last_read_token)) \
      { \
	int i; \
	for (i = 0; word_token_alist[i].word != (char *)NULL; i++) \
	  if (STREQ (tok, word_token_alist[i].word)) \
	    { \
	      if ((parser_state & PST_CASEPAT) && (word_token_alist[i].token != ESAC)) \
		break; \
	      if (word_token_alist[i].token == TIME) \
		break; \
	      if (word_token_alist[i].token == ESAC) \
		parser_state &= ~(PST_CASEPAT|PST_CASESTMT); \
	      else if (word_token_alist[i].token == CASE) \
		parser_state |= PST_CASESTMT; \
	      else if (word_token_alist[i].token == COND_END) \
		parser_state &= ~(PST_CONDCMD|PST_CONDEXPR); \
	      else if (word_token_alist[i].token == COND_START) \
		parser_state |= PST_CONDCMD; \
	      else if (word_token_alist[i].token == '{') \
		open_brace_count++; \
	      else if (word_token_alist[i].token == '}' && open_brace_count) \
		open_brace_count--; \
	      return (word_token_alist[i].token); \
	    } \
      } \
  } while (0)

#if defined (ALIAS)

    /* OK, we have a token.  Let's try to alias expand it, if (and only if)
       it's eligible.

       It is eligible for expansion if EXPAND_ALIASES is set, and
       the token is unquoted and the last token read was a command
       separator (or expand_next_token is set), and we are currently
       processing an alias (pushed_string_list is non-empty) and this
       token is not the same as the current or any previously
       processed alias.

       Special cases that disqualify:
	 In a pattern list in a case statement (parser_state & PST_CASEPAT). */
static int
alias_expand_token (tokstr)
     char *tokstr;
{
  char *expanded;
  alias_t *ap;

  if (((parser_state & PST_ALEXPNEXT) || command_token_position (last_read_token)) &&
	(parser_state & PST_CASEPAT) == 0)
    {
      ap = find_alias (tokstr);

      /* Currently expanding this token. */
      if (ap && (ap->flags & AL_BEINGEXPANDED))
	return (NO_EXPANSION);

      expanded = ap ? savestring (ap->value) : (char *)NULL;
      if (expanded)
	{
	  push_string (expanded, ap->flags & AL_EXPANDNEXT, ap);
	  return (RE_READ_TOKEN);
	}
      else
	/* This is an eligible token that does not have an expansion. */
	return (NO_EXPANSION);
    }
  return (NO_EXPANSION);
}
#endif /* ALIAS */

static int
time_command_acceptable ()
{
#if defined (COMMAND_TIMING)
  switch (last_read_token)
    {
    case 0:
    case ';':
    case '\n':
    case AND_AND:
    case OR_OR:
    case '&':
    case DO:
    case THEN:
    case ELSE:
    case '{':		/* } */
    case '(':		/* ) */
      return 1;
    default:
      return 0;
    }
#else
  return 0;
#endif /* COMMAND_TIMING */
}

/* Handle special cases of token recognition:
	IN is recognized if the last token was WORD and the token
	before that was FOR or CASE or SELECT.

	DO is recognized if the last token was WORD and the token
	before that was FOR or SELECT.

	ESAC is recognized if the last token caused `esacs_needed_count'
	to be set

	`{' is recognized if the last token as WORD and the token
	before that was FUNCTION, or if we just parsed an arithmetic
	`for' command.

	`}' is recognized if there is an unclosed `{' present.

	`-p' is returned as TIMEOPT if the last read token was TIME.

	']]' is returned as COND_END if the parser is currently parsing
	a conditional expression ((parser_state & PST_CONDEXPR) != 0)

	`time' is returned as TIME if and only if it is immediately
	preceded by one of `;', `\n', `||', `&&', or `&'.
*/

static int
special_case_tokens (tokstr)
     char *tokstr;
{
  if ((last_read_token == WORD) &&
#if defined (SELECT_COMMAND)
      ((token_before_that == FOR) || (token_before_that == CASE) || (token_before_that == SELECT)) &&
#else
      ((token_before_that == FOR) || (token_before_that == CASE)) &&
#endif
      (tokstr[0] == 'i' && tokstr[1] == 'n' && tokstr[2] == 0))
    {
      if (token_before_that == CASE)
	{
	  parser_state |= PST_CASEPAT;
	  esacs_needed_count++;
	}
      return (IN);
    }

  if (last_read_token == WORD &&
#if defined (SELECT_COMMAND)
      (token_before_that == FOR || token_before_that == SELECT) &&
#else
      (token_before_that == FOR) &&
#endif
      (tokstr[0] == 'd' && tokstr[1] == 'o' && tokstr[2] == '\0'))
    return (DO);

  /* Ditto for ESAC in the CASE case.
     Specifically, this handles "case word in esac", which is a legal
     construct, certainly because someone will pass an empty arg to the
     case construct, and we don't want it to barf.  Of course, we should
     insist that the case construct has at least one pattern in it, but
     the designers disagree. */
  if (esacs_needed_count)
    {
      esacs_needed_count--;
      if (STREQ (tokstr, "esac"))
	{
	  parser_state &= ~PST_CASEPAT;
	  return (ESAC);
	}
    }

  /* The start of a shell function definition. */
  if (parser_state & PST_ALLOWOPNBRC)
    {
      parser_state &= ~PST_ALLOWOPNBRC;
      if (tokstr[0] == '{' && tokstr[1] == '\0')		/* } */
	{
	  open_brace_count++;
	  function_bstart = line_number;
	  return ('{');					/* } */
	}
    }

  /* We allow a `do' after a for ((...)) without an intervening
     list_terminator */
  if (last_read_token == ARITH_FOR_EXPRS && tokstr[0] == 'd' && tokstr[1] == 'o' && !tokstr[2])
    return (DO);
  if (last_read_token == ARITH_FOR_EXPRS && tokstr[0] == '{' && tokstr[1] == '\0')	/* } */
    {
      open_brace_count++;
      return ('{');			/* } */
    }

  if (open_brace_count && reserved_word_acceptable (last_read_token) && tokstr[0] == '}' && !tokstr[1])
    {
      open_brace_count--;		/* { */
      return ('}');
    }

#if defined (COMMAND_TIMING)
  /* Handle -p after `time'. */
  if (last_read_token == TIME && tokstr[0] == '-' && tokstr[1] == 'p' && !tokstr[2])
    return (TIMEOPT);
#endif

#if defined (COMMAND_TIMING)
  if (STREQ (token, "time") && ((parser_state & PST_CASEPAT) == 0) && time_command_acceptable ())
    return (TIME);
#endif /* COMMAND_TIMING */

#if defined (COND_COMMAND) /* [[ */
  if ((parser_state & PST_CONDEXPR) && tokstr[0] == ']' && tokstr[1] == ']' && tokstr[2] == '\0')
    return (COND_END);
#endif

  return (-1);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
void
reset_parser ()
{
  dstack.delimiter_depth = 0;	/* No delimiters found so far. */
  open_brace_count = 0;

  parser_state = 0;

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  if (pushed_string_list)
    free_string_list ();
#endif /* ALIAS || DPAREN_ARITHMETIC */

  if (shell_input_line)
    {
      free (shell_input_line);
      shell_input_line = (char *)NULL;
      shell_input_line_size = shell_input_line_index = 0;
    }

  FREE (word_desc_to_read);
  word_desc_to_read = (WORD_DESC *)NULL;

  last_read_token = '\n';
  token_to_read = '\n';
}

/* Read the next token.  Command can be READ (normal operation) or
   RESET (to normalize state). */
static int
read_token (command)
     int command;
{
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */

  if (command == RESET)
    {
      reset_parser ();
      return ('\n');
    }

  if (token_to_read)
    {
      result = token_to_read;
      if (token_to_read == WORD || token_to_read == ASSIGNMENT_WORD)
	{
	  yylval.word = word_desc_to_read;
	  word_desc_to_read = (WORD_DESC *)NULL;
	}
      token_to_read = 0;
      return (result);
    }

#if defined (COND_COMMAND)
  if ((parser_state & (PST_CONDCMD|PST_CONDEXPR)) == PST_CONDCMD)
    {
      cond_lineno = line_number;
      parser_state |= PST_CONDEXPR;
      yylval.command = parse_cond_command ();
      if (cond_token != COND_END)
	{
	  cond_error ();
	  return (-1);
	}
      token_to_read = COND_END;
      parser_state &= ~(PST_CONDEXPR|PST_CONDCMD);
      return (COND_CMD);
    }
#endif

#if defined (ALIAS)
  /* This is a place to jump back to once we have successfully expanded a
     token with an alias and pushed the string with push_string () */
 re_read_token:
#endif /* ALIAS */

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && whitespace (character))
    ;

  if (character == EOF)
    {
      EOF_Reached = 1;
      return (yacc_EOF);
    }

  if MBTEST(character == '#' && (!interactive || interactive_comments))
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      discard_until ('\n');
      shell_getc (0);
      character = '\n';	/* this will take the next if statement and return. */
    }

  if (character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	gather_here_documents ();

#if defined (ALIAS)
      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      return (character);
    }

  /* Shell meta-characters. */
  if MBTEST(shellmeta (character) && ((parser_state & PST_DBLPAREN) == 0))
    {
#if defined (ALIAS)
      /* Turn off alias tokenization iff this character sequence would
	 not leave us ready to read a command. */
      if (character == '<' || character == '>')
	parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      peek_char = shell_getc (1);
      if (character == peek_char)
	{
	  switch (character)
	    {
	    case '<':
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	      peek_char = shell_getc (1);
	      if (peek_char == '-')
		return (LESS_LESS_MINUS);
	      else if (peek_char == '<')
		return (LESS_LESS_LESS);
	      else
		{
		  shell_ungetc (peek_char);
		  return (LESS_LESS);
		}

	    case '>':
	      return (GREATER_GREATER);

	    case ';':
	      parser_state |= PST_CASEPAT;
#if defined (ALIAS)
	      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	      return (SEMI_SEMI);

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
	    case '(':		/* ) */
	      result = parse_dparen (character);
	      if (result == -2)
	        break;
	      else
	        return result;
#endif
	    }
	}
      else if MBTEST(character == '<' && peek_char == '&')
	return (LESS_AND);
      else if MBTEST(character == '>' && peek_char == '&')
	return (GREATER_AND);
      else if MBTEST(character == '<' && peek_char == '>')
	return (LESS_GREATER);
      else if MBTEST(character == '>' && peek_char == '|')
	return (GREATER_BAR);
      else if MBTEST(peek_char == '>' && character == '&')
	return (AND_GREATER);

      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if MBTEST(character == ')' && last_read_token == '(' && token_before_that == WORD)
	{
	  parser_state |= PST_ALLOWOPNBRC;
#if defined (ALIAS)
	  parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	  function_dstart = line_number;
	}

      /* case pattern lists may be preceded by an optional left paren.  If
	 we're not trying to parse a case pattern list, the left paren
	 indicates a subshell. */
      if MBTEST(character == '(' && (parser_state & PST_CASEPAT) == 0) /* ) */
	parser_state |= PST_SUBSHELL;
      /*(*/
      else if MBTEST((parser_state & PST_CASEPAT) && character == ')')
	parser_state &= ~PST_CASEPAT;
      /*(*/
      else if MBTEST((parser_state & PST_SUBSHELL) && character == ')')
	parser_state &= ~PST_SUBSHELL;

#if defined (PROCESS_SUBSTITUTION)
      /* Check for the constructs which introduce process substitution.
	 Shells running in `posix mode' don't do process substitution. */
      if MBTEST(posixly_correct || ((character != '>' && character != '<') || peek_char != '(')) /*)*/
#endif /* PROCESS_SUBSTITUTION */
	return (character);
    }

  /* Hack <&- (close stdin) case.  Also <&N- (dup and close). */
  if MBTEST(character == '-' && (last_read_token == LESS_AND || last_read_token == GREATER_AND))
    return (character);

  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  result = read_token_word (character);
#if defined (ALIAS)
  if (result == RE_READ_TOKEN)
    goto re_read_token;
#endif
  return result;
}

/*
 * Match a $(...) or other grouping construct.  This has to handle embedded
 * quoted strings ('', ``, "") and nested constructs.  It also must handle
 * reprompting the user, if necessary, after reading a newline (unless the
 * P_NONL flag is passed), and returning correct error values if it reads
 * EOF.
 */
#define P_FIRSTCLOSE	0x01
#define P_ALLOWESC	0x02

static char matched_pair_error;
static char *
parse_matched_pair (qc, open, close, lenp, flags)
     int qc;	/* `"' if this construct is within double quotes */
     int open, close;
     int *lenp, flags;
{
  int count, ch, was_dollar;
  int pass_next_character, nestlen, ttranslen, start_lineno;
  char *ret, *nestret, *ttrans;
  int retind, retsize;

  count = 1;
  pass_next_character = was_dollar = 0;

  ret = (char *)xmalloc (retsize = 64);
  retind = 0;

  start_lineno = line_number;
  while (count)
    {
      ch = shell_getc ((qc != '\'' || (flags & P_ALLOWESC)) && pass_next_character == 0);
      if (ch == EOF)
	{
	  free (ret);
	  parser_error (start_lineno, "unexpected EOF while looking for matching `%c'", close);
	  EOF_Reached = 1;	/* XXX */
	  return (&matched_pair_error);
	}

      /* Possible reprompting. */
      if (ch == '\n' && interactive &&
	    (bash_input.type == st_stdin || bash_input.type == st_stream))
	prompt_again ();

      if (pass_next_character)		/* last char was backslash */
	{
	  pass_next_character = 0;
	  if (qc != '\'' && ch == '\n')	/* double-quoted \<newline> disappears. */
	    {
	      if (retind > 0) retind--;	/* swallow previously-added backslash */
	      continue;
	    }

	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  if MBTEST(ch == CTLESC || ch == CTLNUL)
	    ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if MBTEST(ch == CTLESC || ch == CTLNUL)	/* special shell escapes */
	{
	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if MBTEST(ch == close)		/* ending delimiter */
	count--;
#if 1
      /* handle nested ${...} specially. */
      else if MBTEST(open != close && was_dollar && open == '{' && ch == open) /* } */
	count++;
#endif
      else if MBTEST(((flags & P_FIRSTCLOSE) == 0) && ch == open)	/* nested begin */
	count++;

      /* Add this character. */
      RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
      ret[retind++] = ch;

      if (open == '\'')			/* '' inside grouping construct */
	{
	  if MBTEST((flags & P_ALLOWESC) && ch == '\\')
	    pass_next_character++;
	  continue;
	}

      if MBTEST(ch == '\\')			/* backslashes */
	pass_next_character++;

      if (open != close)		/* a grouping construct */
	{
	  if MBTEST(shellquote (ch))
	    {
	      /* '', ``, or "" inside $(...) or other grouping construct. */
	      push_delimiter (dstack, ch);
	      if MBTEST(was_dollar && ch == '\'')	/* $'...' inside group */
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, P_ALLOWESC);
	      else
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, 0);
	      pop_delimiter (dstack);
	      if (nestret == &matched_pair_error)
		{
		  free (ret);
		  return &matched_pair_error;
		}
	      if MBTEST(was_dollar && ch == '\'')
		{
		  /* Translate $'...' here. */
		  ttrans = ansiexpand (nestret, 0, nestlen - 1, &ttranslen);
		  xfree (nestret);
		  nestret = sh_single_quote (ttrans);
		  free (ttrans);
		  nestlen = strlen (nestret);
		  retind -= 2;		/* back up before the $' */
		}
	      else if MBTEST(was_dollar && ch == '"')
		{
		  /* Locale expand $"..." here. */
		  ttrans = localeexpand (nestret, 0, nestlen - 1, start_lineno, &ttranslen);
		  xfree (nestret);
		  nestret = (char *)xmalloc (ttranslen + 3);
		  nestret[0] = '"';
		  strcpy (nestret + 1, ttrans);
		  nestret[ttranslen + 1] = '"';
		  nestret[ttranslen += 2] = '\0';
		  free (ttrans);
		  nestlen = ttranslen;
		  retind -= 2;		/* back up before the $" */
		}

	      if (nestlen)
		{
		  RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
		  strcpy (ret + retind, nestret);
		  retind += nestlen;
		}
	      FREE (nestret);
	    }
	}
      /* Parse an old-style command substitution within double quotes as a
	 single word. */
      /* XXX - sh and ksh93 don't do this - XXX */
      else if MBTEST(open == '"' && ch == '`')
	{
	  nestret = parse_matched_pair (0, '`', '`', &nestlen, 0);
	  if (nestret == &matched_pair_error)
	    {
	      free (ret);
	      return &matched_pair_error;
	    }
	  if (nestlen)
	    {
	      RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
	      strcpy (ret + retind, nestret);
	      retind += nestlen;
	    }
	  FREE (nestret);
	}
      else if MBTEST(was_dollar && (ch == '(' || ch == '{' || ch == '['))	/* ) } ] */
	/* check for $(), $[], or ${} inside quoted string. */
	{
	  if (open == ch)	/* undo previous increment */
	    count--;
	  if (ch == '(')		/* ) */
	    nestret = parse_matched_pair (0, '(', ')', &nestlen, 0);
	  else if (ch == '{')		/* } */
	    nestret = parse_matched_pair (0, '{', '}', &nestlen, P_FIRSTCLOSE);
	  else if (ch == '[')		/* ] */
	    nestret = parse_matched_pair (0, '[', ']', &nestlen, 0);
	  if (nestret == &matched_pair_error)
	    {
	      free (ret);
	      return &matched_pair_error;
	    }
	  if (nestlen)
	    {
	      RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
	      strcpy (ret + retind, nestret);
	      retind += nestlen;
	    }
	  FREE (nestret);
	}
      was_dollar = MBTEST(ch == '$');
    }

  ret[retind] = '\0';
  if (lenp)
    *lenp = retind;
  return ret;
}

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
/* Parse a double-paren construct.  It can be either an arithmetic
   command, an arithmetic `for' command, or a nested subshell.  Returns
   the parsed token, -1 on error, or -2 if we didn't do anything and
   should just go on. */
static int
parse_dparen (c)
     int c;
{
  int cmdtyp, len, sline;
  char *wval, *wv2;
  WORD_DESC *wd;

#if defined (ARITH_FOR_COMMAND)
  if (last_read_token == FOR)
    {
      arith_for_lineno = line_number;
      cmdtyp = parse_arith_cmd (&wval);
      if (cmdtyp == 1)
	{
	  /* parse_arith_cmd adds quotes at the beginning and end
	     of the string it returns; we need to take those out. */
	  len = strlen (wval);
	  wv2 = (char *)xmalloc (len);
	  strncpy (wv2, wval + 1, len - 2);
	  wv2[len - 2] = '\0';
	  wd = make_word (wv2);
	  yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
	  free (wval);
	  free (wv2);
	  return (ARITH_FOR_EXPRS);
	}
      else
	return -1;		/* ERROR */
    }
#endif

#if defined (DPAREN_ARITHMETIC)
  if (reserved_word_acceptable (last_read_token))
    {
      sline = line_number;
      cmdtyp = parse_arith_cmd (&wval);
      if (cmdtyp == 1)	/* arithmetic command */
	{
	  wd = make_word (wval);
	  wd->flags = W_QUOTED;
	  yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
	  free (wval);	/* make_word copies it */
	  return (ARITH_CMD);
	}
      else if (cmdtyp == 0)	/* nested subshell */
	{
	  push_string (wval, 0, (alias_t *)NULL);
	  if ((parser_state & PST_CASEPAT) == 0)
	    parser_state |= PST_SUBSHELL;
	  return (c);
	}
      else			/* ERROR */
	return -1;
    }
#endif

  return -2;			/* XXX */
}

/* We've seen a `(('.  Look for the matching `))'.  If we get it, return 1.
   If not, assume it's a nested subshell for backwards compatibility and
   return 0.  In any case, put the characters we've consumed into a locally-
   allocated buffer and make *ep point to that buffer.  Return -1 on an
   error, for example EOF. */
static int
parse_arith_cmd (ep)
     char **ep;
{
  int exp_lineno, rval, c;
  char *ttok, *tokstr;
  int ttoklen;

  exp_lineno = line_number;
  ttok = parse_matched_pair (0, '(', ')', &ttoklen, 0);
  rval = 1;
  if (ttok == &matched_pair_error)
    return -1;
  /* Check that the next character is the closing right paren.  If
     not, this is a syntax error. ( */
  c = shell_getc (0);
  if MBTEST(c != ')')
    rval = 0;

  tokstr = (char *)xmalloc (ttoklen + 4);

  /* (( ... )) -> "..." */
  tokstr[0] = (rval == 1) ? '"' : '(';
  strncpy (tokstr + 1, ttok, ttoklen - 1);	/* don't copy the final `)' */
  if (rval == 1)
    {
      tokstr[ttoklen] = '"';
      tokstr[ttoklen+1] = '\0';
    }
  else
    {
      tokstr[ttoklen] = ')';
      tokstr[ttoklen+1] = c;
      tokstr[ttoklen+2] = '\0';
    }
  *ep = tokstr;
  FREE (ttok);
  return rval;
}
#endif /* DPAREN_ARITHMETIC || ARITH_FOR_COMMAND */

#if defined (COND_COMMAND)
static void
cond_error ()
{
  char *etext;

  if (EOF_Reached && cond_token != COND_ERROR)		/* [[ */
    parser_error (cond_lineno, "unexpected EOF while looking for `]]'");
  else if (cond_token != COND_ERROR)
    {
      if (etext = error_token_from_token (cond_token))
	{
	  parser_error (cond_lineno, "syntax error in conditional expression: unexpected token `%s'", etext);
	  free (etext);
	}
      else
	parser_error (cond_lineno, "syntax error in conditional expression");
    }
}

static COND_COM *
cond_expr ()
{
  return (cond_or ());  
}

static COND_COM *
cond_or ()
{
  COND_COM *l, *r;

  l = cond_and ();
  if (cond_token == OR_OR)
    {
      r = cond_or ();
      l = make_cond_node (COND_OR, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static COND_COM *
cond_and ()
{
  COND_COM *l, *r;

  l = cond_term ();
  if (cond_token == AND_AND)
    {
      r = cond_and ();
      l = make_cond_node (COND_AND, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static int
cond_skip_newlines ()
{
  while ((cond_token = read_token (READ)) == '\n')
    {
      if (interactive && (bash_input.type == st_stdin || bash_input.type == st_stream))
	prompt_again ();
    }
  return (cond_token);
}

#define COND_RETURN_ERROR() \
  do { cond_token = COND_ERROR; return ((COND_COM *)NULL); } while (0)

static COND_COM *
cond_term ()
{
  WORD_DESC *op;
  COND_COM *term, *tleft, *tright;
  int tok, lineno;
  char *etext;

  /* Read a token.  It can be a left paren, a `!', a unary operator, or a
     word that should be the first argument of a binary operator.  Start by
     skipping newlines, since this is a compound command. */
  tok = cond_skip_newlines ();
  lineno = line_number;
  if (tok == COND_END)
    {
      COND_RETURN_ERROR ();
    }
  else if (tok == '(')
    {
      term = cond_expr ();
      if (cond_token != ')')
	{
	  if (term)
	    dispose_cond_node (term);		/* ( */
	  if (etext = error_token_from_token (cond_token))
	    {
	      parser_error (lineno, "unexpected token `%s', expected `)'", etext);
	      free (etext);
	    }
	  else
	    parser_error (lineno, "expected `)'");
	  COND_RETURN_ERROR ();
	}
      term = make_cond_node (COND_EXPR, (WORD_DESC *)NULL, term, (COND_COM *)NULL);
      (void)cond_skip_newlines ();
    }
  else if (tok == BANG || (tok == WORD && (yylval.word->word[0] == '!' && yylval.word->word[1] == '\0')))
    {
      if (tok == WORD)
	dispose_word (yylval.word);	/* not needed */
      term = cond_term ();
      if (term)
	term->flags |= CMD_INVERT_RETURN;
    }
  else if (tok == WORD && test_unop (yylval.word->word))
    {
      op = yylval.word;
      tok = read_token (READ);
      if (tok == WORD)
	{
	  tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	}
      else
	{
	  dispose_word (op);
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, "unexpected argument `%s' to conditional unary operator", etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, "unexpected argument to conditional unary operator");
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else if (tok == WORD)		/* left argument to binary operator */
    {
      /* lhs */
      tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);

      /* binop */
      tok = read_token (READ);
      if (tok == WORD && test_binop (yylval.word->word))
	op = yylval.word;
      else if (tok == '<' || tok == '>')
	op = make_word_from_token (tok);  /* ( */
      /* There should be a check before blindly accepting the `)' that we have
	 seen the opening `('. */
      else if (tok == COND_END || tok == AND_AND || tok == OR_OR || tok == ')')
	{
	  /* Special case.  [[ x ]] is equivalent to [[ -n x ]], just like
	     the test command.  Similarly for [[ x && expr ]] or
	     [[ x || expr ]] or [[ (x) ]]. */
	  op = make_word ("-n");
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	  cond_token = tok;
	  return (term);
	}
      else
	{
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, "unexpected token `%s', conditional binary operator expected", etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, "conditional binary operator expected");
	  dispose_cond_node (tleft);
	  COND_RETURN_ERROR ();
	}

      /* rhs */
      tok = read_token (READ);
      if (tok == WORD)
	{
	  tright = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_BINARY, op, tleft, tright);
	}
      else
	{
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, "unexpected argument `%s' to conditional binary operator", etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, "unexpected argument to conditional binary operator");
	  dispose_cond_node (tleft);
	  dispose_word (op);
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else
    {
      if (tok < 256)
	parser_error (line_number, "unexpected token `%c' in conditional command", tok);
      else if (etext = error_token_from_token (tok))
	{
	  parser_error (line_number, "unexpected token `%s' in conditional command", etext);
	  free (etext);
	}
      else
	parser_error (line_number, "unexpected token %d in conditional command", tok);
      COND_RETURN_ERROR ();
    }
  return (term);
}      

/* This is kind of bogus -- we slip a mini recursive-descent parser in
   here to handle the conditional statement syntax. */
static COMMAND *
parse_cond_command ()
{
  COND_COM *cexp;

  cexp = cond_expr ();
  return (make_cond_command (cexp));
}
#endif

#if defined (ARRAY_VARS)
/* When this is called, it's guaranteed that we don't care about anything
   in t beyond i.  We do save and restore the chars, though. */
static int
token_is_assignment (t, i)
     char *t;
     int i;
{
  unsigned char c, c1;
  int r;

  c = t[i]; c1 = t[i+1];
  t[i] = '='; t[i+1] = '\0';
  r = assignment (t);
  t[i] = c; t[i+1] = c1;
  return r;
}

static int
token_is_ident (t, i)
     char *t;
     int i;
{
  unsigned char c;
  int r;

  c = t[i];
  t[i] = '\0';
  r = legal_identifier (t);
  t[i] = c;
  return r;
}
#endif

static int
read_token_word (character)
     int character;
{
  /* The value for YYLVAL when a WORD is read. */
  WORD_DESC *the_word;

  /* Index into the token that we are building. */
  int token_index;

  /* ALL_DIGITS becomes zero when we see a non-digit. */
  int all_digit_token;

  /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
  int dollar_present;

  /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
  int quoted;

  /* Non-zero means to ignore the value of the next character, and just
     to add it no matter what. */
 int pass_next_character;

  /* The current delimiting character. */
  int cd;
  int result, peek_char;
  char *ttok, *ttrans;
  int ttoklen, ttranslen;
  intmax_t lvalue;

  if (token_buffer_size < TOKEN_DEFAULT_INITIAL_SIZE)
    token = (char *)xrealloc (token, token_buffer_size = TOKEN_DEFAULT_INITIAL_SIZE);

  token_index = 0;
  all_digit_token = DIGIT (character);
  dollar_present = quoted = pass_next_character = 0;

  for (;;)
    {
      if (character == EOF)
	goto got_token;

      if (pass_next_character)
	{
	  pass_next_character = 0;
	  goto got_character;
	}

      cd = current_delimiter (dstack);

      /* Handle backslashes.  Quote lots of things when not inside of
	 double-quotes, quote some things inside of double-quotes. */
      if MBTEST(character == '\\')
	{
	  peek_char = shell_getc (0);

	  /* Backslash-newline is ignored in all cases except
	     when quoted with single quotes. */
	  if (peek_char == '\n')
	    {
	      character = '\n';
	      goto next_character;
	    }
	  else
	    {
	      shell_ungetc (peek_char);

	      /* If the next character is to be quoted, note it now. */
	      if (cd == 0 || cd == '`' ||
		  (cd == '"' && peek_char >= 0 && (sh_syntaxtab[peek_char] & CBSDQUOTE)))
		pass_next_character++;

	      quoted = 1;
	      goto got_character;
	    }
	}

      /* Parse a matched pair of quote characters. */
      if MBTEST(shellquote (character))
	{
	  push_delimiter (dstack, character);
	  ttok = parse_matched_pair (character, character, character, &ttoklen, 0);
	  pop_delimiter (dstack);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size, TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  all_digit_token = 0;
	  quoted = 1;
	  dollar_present |= (character == '"' && strchr (ttok, '$') != 0);
	  FREE (ttok);
	  goto next_character;
	}

#ifdef EXTENDED_GLOB
      /* Parse a ksh-style extended pattern matching specification. */
      if (extended_glob && PATTERN_CHAR (character))
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '(')		/* ) */
	    {
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = all_digit_token = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif /* EXTENDED_GLOB */

      /* If the delimiter character is not single quote, parse some of
	 the shell expansions that must be read as a single word. */
      if (shellexp (character))
	{
	  peek_char = shell_getc (1);
	  /* $(...), <(...), >(...), $((...)), ${...}, and $[...] constructs */
	  if MBTEST(peek_char == '(' || \
		((peek_char == '{' || peek_char == '[') && character == '$'))	/* ) ] } */
	    {
	      if (peek_char == '{')		/* } */
		ttok = parse_matched_pair (cd, '{', '}', &ttoklen, P_FIRSTCLOSE);
	      else if (peek_char == '(')		/* ) */
		{
		  /* XXX - push and pop the `(' as a delimiter for use by
		     the command-oriented-history code.  This way newlines
		     appearing in the $(...) string get added to the
		     history literally rather than causing a possibly-
		     incorrect `;' to be added. ) */
		  push_delimiter (dstack, peek_char);
		  ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
		  pop_delimiter (dstack);
		}
	      else
		ttok = parse_matched_pair (cd, '[', ']', &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  /* This handles $'...' and $"..." new-style quoted strings. */
	  else if MBTEST(character == '$' && (peek_char == '\'' || peek_char == '"'))
	    {
	      int first_line;

	      first_line = line_number;
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (peek_char, peek_char, peek_char,
					 &ttoklen,
					 (peek_char == '\'') ? P_ALLOWESC : 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;
	      if (peek_char == '\'')
		{
		  ttrans = ansiexpand (ttok, 0, ttoklen - 1, &ttranslen);
		  free (ttok);
		  /* Insert the single quotes and correctly quote any
		     embedded single quotes (allowed because P_ALLOWESC was
		     passed to parse_matched_pair). */
		  ttok = sh_single_quote (ttrans);
		  free (ttrans);
		  ttrans = ttok;
		  ttranslen = strlen (ttrans);
		}
	      else
		{
		  /* Try to locale-expand the converted string. */
		  ttrans = localeexpand (ttok, 0, ttoklen - 1, first_line, &ttranslen);
		  free (ttok);

		  /* Add the double quotes back */
		  ttok = (char *)xmalloc (ttranslen + 3);
		  ttok[0] = '"';
		  strcpy (ttok + 1, ttrans);
		  ttok[ttranslen + 1] = '"';
		  ttok[ttranslen += 2] = '\0';
		  free (ttrans);
		  ttrans = ttok;
		}

	      RESIZE_MALLOCED_BUFFER (token, token_index, ttranslen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttrans);
	      token_index += ttranslen;
	      FREE (ttrans);
	      quoted = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  /* This could eventually be extended to recognize all of the
	     shell's single-character parameter expansions, and set flags.*/
	  else if MBTEST(character == '$' && peek_char == '$')
	    {
	      ttok = (char *)xmalloc (3);
	      ttok[0] = ttok[1] = '$';
	      ttok[2] = '\0';
	      RESIZE_MALLOCED_BUFFER (token, token_index, 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttok);
	      token_index += 2;
	      dollar_present = 1;
	      all_digit_token = 0;
	      FREE (ttok);
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}

#if defined (ARRAY_VARS)
      /* Identify possible array subscript assignment; match [...] */
      else if MBTEST(character == '[' && token_index > 0 && assignment_acceptable (last_read_token) && token_is_ident (token, token_index))	/* ] */
        {
	  ttok = parse_matched_pair (cd, '[', ']', &ttoklen, 0);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size,
				  TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  FREE (ttok);
	  all_digit_token = 0;
	  goto next_character;
        }
      /* Identify possible compound array variable assignment. */
      else if MBTEST(character == '=' && token_index > 0 && token_is_assignment (token, token_index))
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '(')		/* ) */
	    {
	      ttok = parse_compound_assignment (&ttoklen);

	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 4,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);

	      token[token_index++] = '=';
	      token[token_index++] = '(';
	      if (ttok)
		{
		  strcpy (token + token_index, ttok);
		  token_index += ttoklen;
		}
	      token[token_index++] = ')';
	      FREE (ttok);
	      all_digit_token = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif

      /* When not parsing a multi-character word construct, shell meta-
	 characters break words. */
      if MBTEST(shellbreak (character))
	{
	  shell_ungetc (character);
	  goto got_token;
	}

    got_character:

      all_digit_token &= DIGIT (character);
      dollar_present |= character == '$';

      if (character == CTLESC || character == CTLNUL)
	token[token_index++] = CTLESC;

      token[token_index++] = character;

      RESIZE_MALLOCED_BUFFER (token, token_index, 1, token_buffer_size,
			      TOKEN_DEFAULT_GROW_SIZE);

    next_character:
      if (character == '\n' && interactive &&
	   (bash_input.type == st_stdin || bash_input.type == st_stream))
	prompt_again ();

      /* We want to remove quoted newlines (that is, a \<newline> pair)
	 unless we are within single quotes or pass_next_character is
	 set (the shell equivalent of literal-next). */
      cd = current_delimiter (dstack);
      character = shell_getc (cd != '\'' && pass_next_character == 0);
    }	/* end for (;;) */

got_token:

  token[token_index] = '\0';

  /* Check to see what thing we should return.  If the last_read_token
     is a `<', or a `&', or the character which ended this token is
     a '>' or '<', then, and ONLY then, is this input token a NUMBER.
     Otherwise, it is just a word, and should be returned as such. */
  if MBTEST(all_digit_token && (character == '<' || character == '>' || \
		    last_read_token == LESS_AND || \
		    last_read_token == GREATER_AND))
      {
	if (legal_number (token, &lvalue) && (int)lvalue == lvalue)
	  yylval.number = lvalue;
	else
	  yylval.number = -1;
	return (NUMBER);
      }

  /* Check for special case tokens. */
  result = (last_shell_getc_is_singlebyte) ? special_case_tokens (token) : -1;
  if (result >= 0)
    return result;

#if defined (ALIAS)
  /* Posix.2 does not allow reserved words to be aliased, so check for all
     of them, including special cases, before expanding the current token
     as an alias. */
  if MBTEST(posixly_correct)
    CHECK_FOR_RESERVED_WORD (token);

  /* Aliases are expanded iff EXPAND_ALIASES is non-zero, and quoting
     inhibits alias expansion. */
  if (expand_aliases && quoted == 0)
    {
      result = alias_expand_token (token);
      if (result == RE_READ_TOKEN)
	return (RE_READ_TOKEN);
      else if (result == NO_EXPANSION)
	parser_state &= ~PST_ALEXPNEXT;
    }

  /* If not in Posix.2 mode, check for reserved words after alias
     expansion. */
  if MBTEST(posixly_correct == 0)
#endif
    CHECK_FOR_RESERVED_WORD (token);

  the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
  the_word->word = (char *)xmalloc (1 + token_index);
  the_word->flags = 0;
  strcpy (the_word->word, token);
  if (dollar_present)
    the_word->flags |= W_HASDOLLAR;
  if (quoted)
    the_word->flags |= W_QUOTED;
  /* A word is an assignment if it appears at the beginning of a
     simple command, or after another assignment word.  This is
     context-dependent, so it cannot be handled in the grammar. */
  if (assignment (token))
    {
      the_word->flags |= W_ASSIGNMENT;
      /* Don't perform word splitting on assignment statements. */
      if (assignment_acceptable (last_read_token))
	the_word->flags |= W_NOSPLIT;
    }

  yylval.word = the_word;

  result = ((the_word->flags & (W_ASSIGNMENT|W_NOSPLIT)) == (W_ASSIGNMENT|W_NOSPLIT))
		? ASSIGNMENT_WORD : WORD;

  if (last_read_token == FUNCTION)
    {
      parser_state |= PST_ALLOWOPNBRC;
      function_dstart = line_number;
    }

  return (result);
}

/* Return 1 if TOKSYM is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (toksym)
     int toksym;
{
  switch (toksym)
    {
    case '\n':
    case ';':
    case '(':
    case ')':
    case '|':
    case '&':
    case '{':
    case '}':		/* XXX */
    case AND_AND:
    case BANG:
    case DO:
    case DONE:
    case ELIF:
    case ELSE:
    case ESAC:
    case FI:
    case IF:
    case OR_OR:
    case SEMI_SEMI:
    case THEN:
    case TIME:
    case TIMEOPT:
    case UNTIL:
    case WHILE:
    case 0:
      return 1;
    default:
      return 0;
    }
}
    
/* Return the index of TOKEN in the alist of reserved words, or -1 if
   TOKEN is not a shell reserved word. */
int
find_reserved_word (tokstr)
     char *tokstr;
{
  int i;
  for (i = 0; word_token_alist[i].word; i++)
    if (STREQ (tokstr, word_token_alist[i].word))
      return i;
  return -1;
}

#if 0
#if defined (READLINE)
/* Called after each time readline is called.  This insures that whatever
   the new prompt string is gets propagated to readline's local prompt
   variable. */
static void
reset_readline_prompt ()
{
  char *temp_prompt;

  if (prompt_string_pointer)
    {
      temp_prompt = (*prompt_string_pointer)
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

      if (temp_prompt == 0)
	{
	  temp_prompt = (char *)xmalloc (1);
	  temp_prompt[0] = '\0';
	}

      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
}
#endif /* READLINE */
#endif /* 0 */

#if defined (HISTORY)
/* A list of tokens which can be followed by newlines, but not by
   semi-colons.  When concatenating multiple lines of history, the
   newline separator for such tokens is replaced with a space. */
static int no_semi_successors[] = {
  '\n', '{', '(', ')', ';', '&', '|',
  CASE, DO, ELSE, IF, SEMI_SEMI, THEN, UNTIL, WHILE, AND_AND, OR_OR, IN,
  0
};

/* If we are not within a delimited expression, try to be smart
   about which separators can be semi-colons and which must be
   newlines.  Returns the string that should be added into the
   history entry. */
char *
history_delimiting_chars ()
{
  register int i;

  if (dstack.delimiter_depth != 0)
    return ("\n");
    
  /* First, handle some special cases. */
  /*(*/
  /* If we just read `()', assume it's a function definition, and don't
     add a semicolon.  If the token before the `)' was not `(', and we're
     not in the midst of parsing a case statement, assume it's a
     parenthesized command and add the semicolon. */
  /*)(*/
  if (token_before_that == ')')
    {
      if (two_tokens_ago == '(')	/*)*/	/* function def */
	return " ";
      /* This does not work for subshells inside case statement
	 command lists.  It's a suboptimal solution. */
      else if (parser_state & PST_CASESTMT)	/* case statement pattern */
	return " ";
      else	
	return "; ";				/* (...) subshell */
    }
  else if (token_before_that == WORD && two_tokens_ago == FUNCTION)
    return " ";		/* function def using `function name' without `()' */

  else if (token_before_that == WORD && two_tokens_ago == FOR)
    {
      /* Tricky.  `for i\nin ...' should not have a semicolon, but
	 `for i\ndo ...' should.  We do what we can. */
      for (i = shell_input_line_index; whitespace(shell_input_line[i]); i++)
	;
      if (shell_input_line[i] && shell_input_line[i] == 'i' && shell_input_line[i+1] == 'n')
	return " ";
      return ";";
    }

  for (i = 0; no_semi_successors[i]; i++)
    {
      if (token_before_that == no_semi_successors[i])
	return (" ");
    }

  return ("; ");
}
#endif /* HISTORY */

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
static void
prompt_again ()
{
  char *temp_prompt;

  if (!interactive)	/* XXX */
    return;

  ps1_prompt = get_string_value ("PS1");
  ps2_prompt = get_string_value ("PS2");

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  temp_prompt = *prompt_string_pointer
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

  if (temp_prompt == 0)
    {
      temp_prompt = (char *)xmalloc (1);
      temp_prompt[0] = '\0';
    }

  current_prompt_string = *prompt_string_pointer;
  prompt_string_pointer = &ps2_prompt;

#if defined (READLINE)
  if (!no_line_editing)
    {
      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
  else
#endif	/* READLINE */
    {
      FREE (current_decoded_prompt);
      current_decoded_prompt = temp_prompt;
    }
}

int
get_current_prompt_level ()
{
  return ((current_prompt_string && current_prompt_string == ps2_prompt) ? 2 : 1);
}

void
set_current_prompt_level (x)
     int x;
{
  prompt_string_pointer = (x == 2) ? &ps2_prompt : &ps1_prompt;
  current_prompt_string = *prompt_string_pointer;
}
      
static void
print_prompt ()
{
  fprintf (stderr, "%s", current_decoded_prompt);
  fflush (stderr);
}

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:

	\a	bell (ascii 07)
	\d	the date in Day Mon Date format
	\e	escape (ascii 033)
	\h	the hostname up to the first `.'
	\H	the hostname
	\j	the number of active jobs
	\l	the basename of the shell's tty device name
	\n	CRLF
	\r	CR
	\s	the name of the shell
	\t	the time in 24-hour hh:mm:ss format
	\T	the time in 12-hour hh:mm:ss format
	\@	the time in 12-hour hh:mm am/pm format
	\A	the time in 24-hour hh:mm format
	\D{fmt}	the result of passing FMT to strftime(3)
	\u	your username
	\v	the version of bash (e.g., 2.00)
	\V	the release of bash, version + patchlevel (e.g., 2.00.0)
	\w	the current working directory
	\W	the last element of $PWD
	\!	the history number of this command
	\#	the command number of this command
	\$	a $ or a # if you are root
	\nnn	character code nnn in octal
	\\	a backslash
	\[	begin a sequence of non-printing chars
	\]	end a sequence of non-printing chars
*/
#define PROMPT_GROWTH 48
char *
decode_prompt_string (string)
     char *string;
{
  WORD_LIST *list;
  char *result, *t;
  struct dstack save_dstack;
  int last_exit_value;
#if defined (PROMPT_STRING_DECODE)
  int result_size, result_index;
  int c, n;
  char *temp, octal_string[4];
  struct tm *tm;  
  time_t the_time;
  char timebuf[128];
  char *timefmt;

  result = (char *)xmalloc (result_size = PROMPT_GROWTH);
  result[result_index = 0] = 0;
  temp = (char *)NULL;

  while (c = *string++)
    {
      if (posixly_correct && c == '!')
	{
	  if (*string == '!')
	    {
	      temp = savestring ("!");
	      goto add_string;
	    }
	  else
	    {
#if !defined (HISTORY)
		temp = savestring ("1");
#else /* HISTORY */
		temp = itos (history_number ());
#endif /* HISTORY */
		string--;	/* add_string increments string again. */
		goto add_string;
	    }
	}
      if (c == '\\')
	{
	  c = *string;

	  switch (c)
	    {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	      strncpy (octal_string, string, 3);
	      octal_string[3] = '\0';

	      n = read_octal (octal_string);
	      temp = (char *)xmalloc (3);

	      if (n == CTLESC || n == CTLNUL)
		{
		  temp[0] = CTLESC;
		  temp[1] = n;
		  temp[2] = '\0';
		}
	      else if (n == -1)
		{
		  temp[0] = '\\';
		  temp[1] = '\0';
		}
	      else
		{
		  temp[0] = n;
		  temp[1] = '\0';
		}

	      for (c = 0; n != -1 && c < 3 && ISOCTAL (*string); c++)
		string++;

	      c = 0;		/* tested at add_string: */
	      goto add_string;

	    case 'd':
	    case 't':
	    case 'T':
	    case '@':
	    case 'A':
	      /* Make the current time/date into a string. */
	      (void) time (&the_time);
	      tm = localtime (&the_time);

	      if (c == 'd')
		n = strftime (timebuf, sizeof (timebuf), "%a %b %d", tm);
	      else if (c == 't')
		n = strftime (timebuf, sizeof (timebuf), "%H:%M:%S", tm);
	      else if (c == 'T')
		n = strftime (timebuf, sizeof (timebuf), "%I:%M:%S", tm);
	      else if (c == '@')
		n = strftime (timebuf, sizeof (timebuf), "%I:%M %p", tm);
	      else if (c == 'A')
		n = strftime (timebuf, sizeof (timebuf), "%H:%M", tm);

	      timebuf[sizeof(timebuf) - 1] = '\0';
	      temp = savestring (timebuf);
	      goto add_string;

	    case 'D':		/* strftime format */
	      if (string[1] != '{')		/* } */
		goto not_escape;

	      (void) time (&the_time);
	      tm = localtime (&the_time);
	      string += 2;			/* skip { */
	      timefmt = xmalloc (strlen (string) + 3);
	      for (t = timefmt; *string && *string != '}'; )
		*t++ = *string++;
	      *t = '\0';
	      c = *string;	/* tested at add_string */
	      if (timefmt[0] == '\0')
		{
		  timefmt[0] = '%';
		  timefmt[1] = 'X';	/* locale-specific current time */
		  timefmt[2] = '\0';
		}
	      n = strftime (timebuf, sizeof (timebuf), timefmt, tm);
	      free (timefmt);

	      timebuf[sizeof(timebuf) - 1] = '\0';
	      if (promptvars || posixly_correct)
		/* Make sure that expand_prompt_string is called with a
		   second argument of Q_DOUBLE_QUOTES if we use this
		   function here. */
		temp = sh_backslash_quote_for_double_quotes (timebuf);
	      else
		temp = savestring (timebuf);
	      goto add_string;
	      
	    case 'n':
	      temp = (char *)xmalloc (3);
	      temp[0] = no_line_editing ? '\n' : '\r';
	      temp[1] = no_line_editing ? '\0' : '\n';
	      temp[2] = '\0';
	      goto add_string;

	    case 's':
	      temp = base_pathname (shell_name);
	      temp = savestring (temp);
	      goto add_string;

	    case 'v':
	    case 'V':
	      temp = (char *)xmalloc (16);
	      if (c == 'v')
		strcpy (temp, dist_version);
	      else
		sprintf (temp, "%s.%d", dist_version, patch_level);
	      goto add_string;

	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more efficient. */
		char t_string[PATH_MAX];
		int tlen;

		temp = get_string_value ("PWD");

		if (temp == 0)
		  {
		    if (getcwd (t_string, sizeof(t_string)) == 0)
		      {
			t_string[0] = '.';
			tlen = 1;
		      }
		    else
		      tlen = strlen (t_string);
		  }
		else
		  {
		    tlen = sizeof (t_string) - 1;
		    strncpy (t_string, temp, tlen);
		  }
		t_string[tlen] = '\0';

#define ROOT_PATH(x)	((x)[0] == '/' && (x)[1] == 0)
#define DOUBLE_SLASH_ROOT(x)	((x)[0] == '/' && (x)[1] == '/' && (x)[2] == 0)
		if (c == 'W')
		  {
		    if (ROOT_PATH (t_string) == 0 && DOUBLE_SLASH_ROOT (t_string) == 0)
		      {
			t = strrchr (t_string, '/');
			if (t)
			  strcpy (t_string, t + 1);
		      }
		  }
#undef ROOT_PATH
#undef DOUBLE_SLASH_ROOT
		else
		  /* polite_directory_format is guaranteed to return a string
		     no longer than PATH_MAX - 1 characters. */
		  strcpy (t_string, polite_directory_format (t_string));

		/* If we're going to be expanding the prompt string later,
		   quote the directory name. */
		if (promptvars || posixly_correct)
		  /* Make sure that expand_prompt_string is called with a
		     second argument of Q_DOUBLE_QUOTES if we use this
		     function here. */
		  temp = sh_backslash_quote_for_double_quotes (t_string);
		else
		  temp = savestring (t_string);

		goto add_string;
	      }

	    case 'u':
	      if (current_user.user_name == 0)
		get_current_user_info ();
	      temp = savestring (current_user.user_name);
	      goto add_string;

	    case 'h':
	    case 'H':
	      temp = savestring (current_host_name);
	      if (c == 'h' && (t = (char *)strchr (temp, '.')))
		*t = '\0';
	      goto add_string;

	    case '#':
	      temp = itos (current_command_number);
	      goto add_string;

	    case '!':
#if !defined (HISTORY)
	      temp = savestring ("1");
#else /* HISTORY */
	      temp = itos (history_number ());
#endif /* HISTORY */
	      goto add_string;

	    case '$':
	      t = temp = (char *)xmalloc (3);
	      if ((promptvars || posixly_correct) && (current_user.euid != 0))
		*t++ = '\\';
	      *t++ = current_user.euid == 0 ? '#' : '$';
	      *t = '\0';
	      goto add_string;

	    case 'j':
	      temp = itos (count_all_jobs ());
	      goto add_string;

	    case 'l':
#if defined (HAVE_TTYNAME)
	      temp = (char *)ttyname (fileno (stdin));
	      t = temp ? base_pathname (temp) : "tty";
	      temp = savestring (t);
#else
	      temp = savestring ("tty");
#endif /* !HAVE_TTYNAME */
	      goto add_string;

#if defined (READLINE)
	    case '[':
	    case ']':
	      temp = (char *)xmalloc (3);
	      temp[0] = '\001';
	      temp[1] = (c == '[') ? RL_PROMPT_START_IGNORE : RL_PROMPT_END_IGNORE;
	      temp[2] = '\0';
	      goto add_string;
#endif /* READLINE */

	    case '\\':
	    case 'a':
	    case 'e':
	    case 'r':
	      temp = (char *)xmalloc (2);
	      if (c == 'a')
		temp[0] = '\07';
	      else if (c == 'e')
		temp[0] = '\033';
	      else if (c == 'r')
		temp[0] = '\r';
	      else			/* (c == '\\') */
	        temp[0] = c;
	      temp[1] = '\0';
	      goto add_string;

	    default:
not_escape:
	      temp = (char *)xmalloc (3);
	      temp[0] = '\\';
	      temp[1] = c;
	      temp[2] = '\0';

	    add_string:
	      if (c)
		string++;
	      result =
		sub_append_string (temp, result, &result_index, &result_size);
	      temp = (char *)NULL; /* Freed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  RESIZE_MALLOCED_BUFFER (result, result_index, 3, result_size, PROMPT_GROWTH);
	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }
#else /* !PROMPT_STRING_DECODE */
  result = savestring (string);
#endif /* !PROMPT_STRING_DECODE */

  /* Save the delimiter stack and point `dstack' to temp space so any
     command substitutions in the prompt string won't result in screwing
     up the parser's quoting state. */
  save_dstack = dstack;
  dstack = temp_dstack;
  dstack.delimiter_depth = 0;

  /* Perform variable and parameter expansion and command substitution on
     the prompt string. */
  if (promptvars || posixly_correct)
    {
      last_exit_value = last_command_exit_value;
      list = expand_prompt_string (result, Q_DOUBLE_QUOTES);
      free (result);
      result = string_list (list);
      dispose_words (list);
      last_command_exit_value = last_exit_value;
    }
  else
    {
      t = dequote_string (result);
      free (result);
      result = t;
    }

  dstack = save_dstack;

  return (result);
}

/************************************************
 *						*
 *		ERROR HANDLING			*
 *						*
 ************************************************/

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
int
yyerror (msg)
     const char *msg;
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
  return (0);
}

static char *
error_token_from_token (token)
     int token;
{
  char *t;

  if (t = find_token_in_alist (token, word_token_alist, 0))
    return t;

  if (t = find_token_in_alist (token, other_token_alist, 0))
    return t;

  t = (char *)NULL;
  /* This stuff is dicy and needs closer inspection */
  switch (current_token)
    {
    case WORD:
    case ASSIGNMENT_WORD:
      if (yylval.word)
	t = savestring (yylval.word->word);
      break;
    case NUMBER:
      t = itos (yylval.number);
      break;
    case ARITH_CMD:
      if (yylval.word_list)
        t = string_list (yylval.word_list);
      break;
    case ARITH_FOR_EXPRS:
      if (yylval.word_list)
	t = string_list_internal (yylval.word_list, " ; ");
      break;
    case COND_CMD:
      t = (char *)NULL;		/* punt */
      break;
    }

  return t;
}

static char *
error_token_from_text ()
{
  char *msg, *t;
  int token_end, i;

  t = shell_input_line;
  i = shell_input_line_index;
  token_end = 0;
  msg = (char *)NULL;

  if (i && t[i] == '\0')
    i--;

  while (i && (whitespace (t[i]) || t[i] == '\n'))
    i--;

  if (i)
    token_end = i + 1;

  while (i && (member (t[i], " \n\t;|&") == 0))
    i--;

  while (i != token_end && (whitespace (t[i]) || t[i] == '\n'))
    i++;

  /* Return our idea of the offending token. */
  if (token_end || (i == 0 && token_end == 0))
    {
      if (token_end)
	msg = substring (t, i, token_end);
      else	/* one-character token */
	{
	  msg = (char *)xmalloc (2);
	  msg[0] = t[i];
	  msg[1] = '\0';
	}
    }

  return (msg);
}

static void
print_offending_line ()
{
  char *msg;
  int token_end;

  msg = savestring (shell_input_line);
  token_end = strlen (msg);
  while (token_end && msg[token_end - 1] == '\n')
    msg[--token_end] = '\0';

  parser_error (line_number, "`%s'", msg);
  free (msg);
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
static void
report_syntax_error (message)
     char *message;
{
  char *msg;

  if (message)
    {
      parser_error (line_number, "%s", message);
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
      last_command_exit_value = EX_USAGE;
      return;
    }

  /* If the line of input we're reading is not null, try to find the
     objectionable token.  First, try to figure out what token the
     parser's complaining about by looking at current_token. */
  if (current_token != 0 && EOF_Reached == 0 && (msg = error_token_from_token (current_token)))
    {
      parser_error (line_number, "syntax error near unexpected token `%s'", msg);
      free (msg);

      if (interactive == 0)
	print_offending_line ();

      last_command_exit_value = EX_USAGE;
      return;
    }

  /* If looking at the current token doesn't prove fruitful, try to find the
     offending token by analyzing the text of the input line near the current
     input line index and report what we find. */
  if (shell_input_line && *shell_input_line)
    {
      msg = error_token_from_text ();
      if (msg)
	{
	  parser_error (line_number, "syntax error near `%s'", msg);
	  free (msg);
	}

      /* If not interactive, print the line containing the error. */
      if (interactive == 0)
        print_offending_line ();
    }
  else
    {
      msg = EOF_Reached ? "syntax error: unexpected end of file" : "syntax error";
      parser_error (line_number, "%s", msg);
      /* When the shell is interactive, this file uses EOF_Reached
	 only for error reporting.  Other mechanisms are used to
	 decide whether or not to exit. */
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
    }

  last_command_exit_value = EX_USAGE;
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command.) */
static void
discard_parser_constructs (error_p)
     int error_p;
{
}

/************************************************
 *						*
 *		EOF HANDLING			*
 *						*
 ************************************************/

/* Do that silly `type "bye" to exit' stuff.  You know, "ignoreeof". */

/* A flag denoting whether or not ignoreeof is set. */
int ignoreeof = 0;

/* The number of times that we have encountered an EOF character without
   another character intervening.  When this gets above the limit, the
   shell terminates. */
int eof_encountered = 0;

/* The limit for eof_encountered. */
int eof_encountered_limit = 10;

/* If we have EOF as the only input unit, this user wants to leave
   the shell.  If the shell is not interactive, then just leave.
   Otherwise, if ignoreeof is set, and we haven't done this the
   required number of times in a row, print a message. */
static void
handle_eof_input_unit ()
{
  if (interactive)
    {
      /* shell.c may use this to decide whether or not to write out the
	 history, among other things.  We use it only for error reporting
	 in this file. */
      if (EOF_Reached)
	EOF_Reached = 0;

      /* If the user wants to "ignore" eof, then let her do so, kind of. */
      if (ignoreeof)
	{
	  if (eof_encountered < eof_encountered_limit)
	    {
	      fprintf (stderr, "Use \"%s\" to leave the shell.\n",
		       login_shell ? "logout" : "exit");
	      eof_encountered++;
	      /* Reset the parsing state. */
	      last_read_token = current_token = '\n';
	      /* Reset the prompt string to be $PS1. */
	      prompt_string_pointer = (char **)NULL;
	      prompt_again ();
	      return;
	    }
	}

      /* In this case EOF should exit the shell.  Do it now. */
      reset_parser ();
      exit_builtin ((WORD_LIST *)NULL);
    }
  else
    {
      /* We don't write history files, etc., for non-interactive shells. */
      EOF_Reached = 1;
    }
}

/************************************************
 *						*
 *	STRING PARSING FUNCTIONS		*
 *						*
 ************************************************/

/* It's very important that these two functions treat the characters
   between ( and ) identically. */

static WORD_LIST parse_string_error;

/* Take a string and run it through the shell parser, returning the
   resultant word list.  Used by compound array assignment. */
WORD_LIST *
parse_string_to_word_list (s, whom)
     char *s;
     const char *whom;
{
  WORD_LIST *wl;
  int tok, orig_current_token, orig_line_number, orig_input_terminator;
  int orig_line_count;
  int old_echo_input, old_expand_aliases;
#if defined (HISTORY)
  int old_remember_on_history, old_history_expansion_inhibited;
#endif

#if defined (HISTORY)
  old_remember_on_history = remember_on_history;
#  if defined (BANG_HISTORY)
  old_history_expansion_inhibited = history_expansion_inhibited;
#  endif
  bash_history_disable ();
#endif

  orig_line_number = line_number;
  orig_line_count = current_command_line_count;
  orig_input_terminator = shell_input_line_terminator;
  old_echo_input = echo_input_at_read;
  old_expand_aliases = expand_aliases;

  push_stream (1);
  last_read_token = WORD;		/* WORD to allow reserved words here */
  current_command_line_count = 0;
  echo_input_at_read = expand_aliases = 0;

  with_input_from_string (s, whom);
  wl = (WORD_LIST *)NULL;
  while ((tok = read_token (READ)) != yacc_EOF)
    {
      if (tok == '\n' && *bash_input.location.string == '\0')
	break;
      if (tok == '\n')		/* Allow newlines in compound assignments */
	continue;
      if (tok != WORD && tok != ASSIGNMENT_WORD)
	{
	  line_number = orig_line_number + line_number - 1;
	  orig_current_token = current_token;
	  current_token = tok;
	  yyerror ((char *)NULL);	/* does the right thing */
	  current_token = orig_current_token;
	  if (wl)
	    dispose_words (wl);
	  wl = &parse_string_error;
	  break;
	}
      wl = make_word_list (yylval.word, wl);
    }
  
  last_read_token = '\n';
  pop_stream ();

#if defined (HISTORY)
  remember_on_history = old_remember_on_history;
#  if defined (BANG_HISTORY)
  history_expansion_inhibited = old_history_expansion_inhibited;
#  endif /* BANG_HISTORY */
#endif /* HISTORY */

  echo_input_at_read = old_echo_input;
  expand_aliases = old_expand_aliases;

  current_command_line_count = orig_line_count;
  shell_input_line_terminator = orig_input_terminator;

  if (wl == &parse_string_error)
    {
      last_command_exit_value = EXECUTION_FAILURE;
      if (interactive_shell == 0 && posixly_correct)
	jump_to_top_level (FORCE_EOF);
      else
	jump_to_top_level (DISCARD);
    }

  return (REVERSE_LIST (wl, WORD_LIST *));
}

static char *
parse_compound_assignment (retlenp)
     int *retlenp;
{
  WORD_LIST *wl, *rl;
  int tok, orig_line_number, orig_token_size;
  char *saved_token, *ret;

  saved_token = token;
  orig_token_size = token_buffer_size;
  orig_line_number = line_number;

  last_read_token = WORD;	/* WORD to allow reserved words here */

  token = (char *)NULL;
  token_buffer_size = 0;

  wl = (WORD_LIST *)NULL;	/* ( */
  while ((tok = read_token (READ)) != ')')
    {
      if (tok == '\n')			/* Allow newlines in compound assignments */
	continue;
      if (tok != WORD && tok != ASSIGNMENT_WORD)
	{
	  current_token = tok;	/* for error reporting */
	  if (tok == yacc_EOF)	/* ( */
	    parser_error (orig_line_number, "unexpected EOF while looking for matching `)'");
	  else
	    yyerror ((char *)NULL);	/* does the right thing */
	  if (wl)
	    dispose_words (wl);
	  wl = &parse_string_error;
	  break;
	}
      wl = make_word_list (yylval.word, wl);
    }

  FREE (token);
  token = saved_token;
  token_buffer_size = orig_token_size;

  if (wl == &parse_string_error)
    {
      last_command_exit_value = EXECUTION_FAILURE;
      last_read_token = '\n';	/* XXX */
      if (interactive_shell == 0 && posixly_correct)
	jump_to_top_level (FORCE_EOF);
      else
	jump_to_top_level (DISCARD);
    }

  last_read_token = WORD;
  if (wl)
    {
      rl = REVERSE_LIST (wl, WORD_LIST *);
      ret = string_list (rl);
      dispose_words (rl);
    }
  else
    ret = (char *)NULL;

  if (retlenp)
    *retlenp = (ret && *ret) ? strlen (ret) : 0;
  return ret;
}

/************************************************
 *						*
 *	MULTIBYTE CHARACTER HANDLING		*
 *						*
 ************************************************/

#if defined (HANDLE_MULTIBYTE)
static void
set_line_mbstate ()
{
  int i, previ, len;
  mbstate_t mbs, prevs;
  size_t mbclen;

  if (shell_input_line == NULL)
    return;
  len = strlen (shell_input_line);	/* XXX - shell_input_line_len ? */
  FREE (shell_input_line_property);
  shell_input_line_property = (char *)xmalloc (len + 1);

  memset (&prevs, '\0', sizeof (mbstate_t));
  for (i = previ = 0; i < len; i++)
    {
      mbs = prevs;

      if (shell_input_line[i] == EOF)
	{
	  int j;
	  for (j = i; j < len; j++)
	    shell_input_line_property[j] = 1;
	  break;
	}

      mbclen = mbrlen (shell_input_line + previ, i - previ + 1, &mbs);
      if (mbclen == 1 || mbclen == (size_t)-1)
	{
	  mbclen = 1;
	  previ = i + 1;
	}
      else if (mbclen == (size_t)-2)
        mbclen = 0;
      else if (mbclen > 1)
	{
	  mbclen = 0;
	  previ = i + 1;
	  prevs = mbs;
	}
      else
	{
	  /* mbrlen doesn't return any other values */
	}

      shell_input_line_property[i] = mbclen;
    }
}
#endif /* HANDLE_MULTIBYTE */
