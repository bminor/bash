
/*  A Bison parser, made from ./parse.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	IF	258
#define	THEN	259
#define	ELSE	260
#define	ELIF	261
#define	FI	262
#define	CASE	263
#define	ESAC	264
#define	FOR	265
#define	SELECT	266
#define	WHILE	267
#define	UNTIL	268
#define	DO	269
#define	DONE	270
#define	FUNCTION	271
#define	IN	272
#define	BANG	273
#define	TIME	274
#define	TIMEOPT	275
#define	WORD	276
#define	ASSIGNMENT_WORD	277
#define	NUMBER	278
#define	AND_AND	279
#define	OR_OR	280
#define	GREATER_GREATER	281
#define	LESS_LESS	282
#define	LESS_AND	283
#define	GREATER_AND	284
#define	SEMI_SEMI	285
#define	LESS_LESS_MINUS	286
#define	AND_GREATER	287
#define	LESS_GREATER	288
#define	GREATER_BAR	289
#define	yacc_EOF	290

#line 21 "./parse.y"

#include "config.h"

#include "bashtypes.h"
#include "bashansi.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (HAVE_LOCALE_H)
#  include <locale.h>
#endif

#include <stdio.h>
#include <signal.h>

#include "memalloc.h"

#include "shell.h"
#include "trap.h"
#include "flags.h"
#include "parser.h"
#include "mailcheck.h"
#include "builtins/common.h"
#include "builtins/builtext.h"

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
#include <sys/param.h>
#include <time.h>
#include "maxpath.h"
#endif /* PROMPT_STRING_DECODE */

#define RE_READ_TOKEN	-99
#define NO_EXPANSION	-100

#define YYDEBUG 0

extern int eof_encountered;
extern int no_line_editing, running_under_emacs;
extern int current_command_number;
extern int interactive, interactive_shell, login_shell;
extern int sourcelevel;
extern int posixly_correct;
extern int last_command_exit_value;
extern int interrupt_immediately;
extern char *shell_name, *current_host_name;
extern char *dist_version;
extern int patch_level;
extern int dump_translatable_strings;
extern Function *last_shell_builtin, *this_shell_builtin;
#if defined (BUFFERED_INPUT)
extern int bash_input_fd_changed;
#endif

/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

static char *ansiexpand ();
static char *localeexpand ();
static int reserved_word_acceptable ();
static int read_token ();
static int yylex ();
static int parse_arith_cmd ();
static int read_token_word ();
static void discard_parser_constructs ();

static void report_syntax_error ();
static void handle_eof_input_unit ();
static void prompt_again ();
#if 0
static void reset_readline_prompt ();
#endif
static void print_prompt ();

extern int yyerror ();

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

static REDIRECTEE redir;

#line 171 "./parse.y"
typedef union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		263
#define	YYFLAG		-32768
#define	YYNTBASE	47

#define YYTRANSLATE(x) ((unsigned)(x) <= 290 ? yytranslate[x] : 78)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    37,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    35,     2,    45,
    46,     2,     2,     2,    42,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    36,    41,
     2,    40,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    43,    39,    44,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    38
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,     8,    10,    12,    15,    18,    21,    25,
    29,    32,    36,    39,    43,    46,    50,    53,    57,    60,
    64,    67,    71,    74,    78,    81,    85,    88,    92,    95,
    99,   102,   105,   109,   111,   113,   115,   117,   120,   122,
   125,   127,   129,   132,   134,   136,   142,   148,   150,   152,
   154,   156,   158,   165,   172,   180,   188,   199,   210,   217,
   224,   232,   240,   251,   262,   269,   277,   284,   290,   297,
   302,   306,   312,   320,   327,   331,   336,   343,   349,   351,
   354,   359,   364,   370,   376,   379,   383,   385,   389,   392,
   394,   397,   401,   405,   409,   414,   419,   424,   429,   434,
   436,   438,   440,   442,   443,   446,   448,   451,   454,   459,
   464,   468,   472,   474,   476,   479,   482,   486,   490,   495,
   497,   499
};

static const short yyrhs[] = {    73,
    37,     0,    37,     0,     1,    37,     0,    38,     0,    21,
     0,    48,    21,     0,    40,    21,     0,    41,    21,     0,
    23,    40,    21,     0,    23,    41,    21,     0,    26,    21,
     0,    23,    26,    21,     0,    27,    21,     0,    23,    27,
    21,     0,    28,    23,     0,    23,    28,    23,     0,    29,
    23,     0,    23,    29,    23,     0,    28,    21,     0,    23,
    28,    21,     0,    29,    21,     0,    23,    29,    21,     0,
    31,    21,     0,    23,    31,    21,     0,    29,    42,     0,
    23,    29,    42,     0,    28,    42,     0,    23,    28,    42,
     0,    32,    21,     0,    23,    33,    21,     0,    33,    21,
     0,    34,    21,     0,    23,    34,    21,     0,    21,     0,
    22,     0,    49,     0,    49,     0,    51,    49,     0,    50,
     0,    52,    50,     0,    52,     0,    54,     0,    54,    51,
     0,    55,     0,    57,     0,    12,    68,    14,    68,    15,
     0,    13,    68,    14,    68,    15,     0,    56,     0,    60,
     0,    59,     0,    61,     0,    58,     0,    10,    21,    72,
    14,    68,    15,     0,    10,    21,    72,    43,    68,    44,
     0,    10,    21,    36,    72,    14,    68,    15,     0,    10,
    21,    36,    72,    43,    68,    44,     0,    10,    21,    72,
    17,    48,    71,    72,    14,    68,    15,     0,    10,    21,
    72,    17,    48,    71,    72,    43,    68,    44,     0,    11,
    21,    72,    14,    67,    15,     0,    11,    21,    72,    43,
    67,    44,     0,    11,    21,    36,    72,    14,    67,    15,
     0,    11,    21,    36,    72,    43,    67,    44,     0,    11,
    21,    72,    17,    48,    71,    72,    14,    67,    15,     0,
    11,    21,    72,    17,    48,    71,    72,    43,    67,    44,
     0,     8,    21,    72,    17,    72,     9,     0,     8,    21,
    72,    17,    65,    72,     9,     0,     8,    21,    72,    17,
    63,     9,     0,    21,    45,    46,    72,    61,     0,    16,
    21,    45,    46,    72,    61,     0,    16,    21,    72,    61,
     0,    45,    68,    46,     0,     3,    68,     4,    68,     7,
     0,     3,    68,     4,    68,     5,    68,     7,     0,     3,
    68,     4,    68,    62,     7,     0,    43,    67,    44,     0,
     6,    68,     4,    68,     0,     6,    68,     4,    68,     5,
    68,     0,     6,    68,     4,    68,    62,     0,    64,     0,
    65,    64,     0,    72,    66,    46,    68,     0,    72,    66,
    46,    72,     0,    72,    45,    66,    46,    68,     0,    72,
    45,    66,    46,    72,     0,    64,    30,     0,    65,    64,
    30,     0,    21,     0,    66,    39,    21,     0,    72,    69,
     0,    67,     0,    72,    70,     0,    70,    37,    72,     0,
    70,    35,    72,     0,    70,    36,    72,     0,    70,    24,
    72,    70,     0,    70,    25,    72,    70,     0,    70,    35,
    72,    70,     0,    70,    36,    72,    70,     0,    70,    37,
    72,    70,     0,    75,     0,    37,     0,    36,     0,    38,
     0,     0,    72,    37,     0,    74,     0,    74,    35,     0,
    74,    36,     0,    74,    24,    72,    74,     0,    74,    25,
    72,    74,     0,    74,    35,    74,     0,    74,    36,    74,
     0,    75,     0,    76,     0,    18,    76,     0,    77,    76,
     0,    77,    18,    76,     0,    18,    77,    76,     0,    76,
    39,    72,    76,     0,    53,     0,    19,     0,    19,    20,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   214,   223,   230,   245,   255,   257,   261,   266,   271,   276,
   281,   286,   291,   297,   303,   308,   313,   318,   323,   328,
   333,   338,   343,   350,   357,   362,   367,   372,   377,   382,
   387,   392,   397,   404,   406,   408,   412,   416,   427,   429,
   433,   435,   437,   466,   468,   470,   472,   474,   476,   478,
   480,   482,   486,   488,   490,   492,   494,   496,   500,   504,
   508,   512,   516,   520,   526,   528,   530,   534,   538,   541,
   545,   549,   551,   553,   558,   562,   564,   566,   570,   571,
   575,   577,   579,   581,   585,   586,   590,   592,   601,   609,
   610,   616,   617,   624,   628,   630,   632,   639,   641,   643,
   647,   648,   649,   652,   653,   662,   668,   677,   685,   687,
   689,   696,   699,   703,   705,   710,   715,   720,   727,   730,
   734,   736
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","IF","THEN",
"ELSE","ELIF","FI","CASE","ESAC","FOR","SELECT","WHILE","UNTIL","DO","DONE",
"FUNCTION","IN","BANG","TIME","TIMEOPT","WORD","ASSIGNMENT_WORD","NUMBER","AND_AND",
"OR_OR","GREATER_GREATER","LESS_LESS","LESS_AND","GREATER_AND","SEMI_SEMI","LESS_LESS_MINUS",
"AND_GREATER","LESS_GREATER","GREATER_BAR","'&'","';'","'\\n'","yacc_EOF","'|'",
"'>'","'<'","'-'","'{'","'}'","'('","')'","inputunit","word_list","redirection",
"simple_command_element","redirection_list","simple_command","command","shell_command",
"for_command","select_command","case_command","function_def","subshell","if_command",
"group_command","elif_clause","case_clause","pattern_list","case_clause_sequence",
"pattern","list","compound_list","list0","list1","list_terminator","newline_list",
"simple_list","simple_list1","pipeline_command","pipeline","timespec", NULL
};
#endif

static const short yyr1[] = {     0,
    47,    47,    47,    47,    48,    48,    49,    49,    49,    49,
    49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
    49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
    49,    49,    49,    50,    50,    50,    51,    51,    52,    52,
    53,    53,    53,    54,    54,    54,    54,    54,    54,    54,
    54,    54,    55,    55,    55,    55,    55,    55,    56,    56,
    56,    56,    56,    56,    57,    57,    57,    58,    58,    58,
    59,    60,    60,    60,    61,    62,    62,    62,    63,    63,
    64,    64,    64,    64,    65,    65,    66,    66,    67,    68,
    68,    69,    69,    69,    70,    70,    70,    70,    70,    70,
    71,    71,    71,    72,    72,    73,    73,    73,    74,    74,
    74,    74,    74,    75,    75,    75,    75,    75,    76,    76,
    77,    77
};

static const short yyr2[] = {     0,
     2,     1,     2,     1,     1,     2,     2,     2,     3,     3,
     2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
     2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
     2,     2,     3,     1,     1,     1,     1,     2,     1,     2,
     1,     1,     2,     1,     1,     5,     5,     1,     1,     1,
     1,     1,     6,     6,     7,     7,    10,    10,     6,     6,
     7,     7,    10,    10,     6,     7,     6,     5,     6,     4,
     3,     5,     7,     6,     3,     4,     6,     5,     1,     2,
     4,     4,     5,     5,     2,     3,     1,     3,     2,     1,
     2,     3,     3,     3,     4,     4,     4,     4,     4,     1,
     1,     1,     1,     0,     2,     1,     2,     2,     4,     4,
     3,     3,     1,     1,     2,     2,     3,     3,     4,     1,
     1,     2
};

static const short yydefact[] = {     0,
     0,   104,     0,     0,     0,   104,   104,     0,     0,   121,
    34,    35,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     2,     4,     0,     0,   104,   104,    36,    39,    41,
   120,    42,    44,    48,    45,    52,    50,    49,    51,     0,
   106,   113,   114,     0,     3,    90,     0,     0,   104,   104,
   104,     0,     0,   104,   115,     0,   122,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    11,    13,    19,
    15,    27,    21,    17,    25,    23,    29,    31,    32,     7,
     8,     0,     0,     0,    34,    40,    37,    43,     1,   104,
   104,   107,   108,   104,     0,   116,   104,   105,    89,    91,
   100,     0,   104,     0,   104,     0,   104,   104,     0,     0,
   118,   104,    12,    14,    20,    16,    28,    22,    18,    26,
    24,    30,    33,     9,    10,    75,     0,    71,    38,     0,
     0,   111,   112,     0,   117,     0,   104,   104,   104,   104,
   104,   104,     0,   104,     0,   104,     0,   104,     0,   104,
     0,     0,   104,    70,     0,   109,   110,     0,     0,   119,
   104,   104,    72,     0,     0,     0,    93,    94,    92,     0,
    79,   104,     0,   104,   104,     0,     5,     0,     0,   104,
   104,     0,     0,     0,    46,    47,     0,    68,     0,     0,
    74,    95,    96,    97,    98,    99,    67,    85,    80,     0,
    65,    87,     0,     0,     0,     0,    53,     6,   102,   101,
   103,   104,    54,     0,     0,    59,   104,    60,    69,    73,
   104,   104,   104,   104,    86,    66,     0,     0,   104,    55,
    56,     0,    61,    62,     0,    76,     0,     0,     0,   104,
    88,    81,    82,   104,   104,   104,   104,   104,    78,    83,
    84,     0,     0,     0,     0,    77,    57,    58,    63,    64,
     0,     0,     0
};

static const short yydefgoto[] = {   261,
   178,    28,    29,    88,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,   164,   170,   171,   172,   204,    46,
    47,    99,   100,   212,    48,    40,   132,   101,    43,    44
};

static const short yypact[] = {   240,
   -25,-32768,    -4,    41,    44,-32768,-32768,    49,   348,     0,
   -31,-32768,   476,    53,    55,     3,    29,    61,    63,    66,
    71,-32768,-32768,    76,    79,-32768,-32768,-32768,-32768,   165,
-32768,   120,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -12,
    89,-32768,    12,   384,-32768,-32768,    99,   276,-32768,    93,
   103,    98,   143,   126,    12,   456,-32768,   127,   151,   153,
    35,    43,   154,   155,   159,   160,   161,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   140,   276,   137,-32768,-32768,-32768,   120,-32768,-32768,
-32768,   312,   312,-32768,   456,    12,-32768,-32768,-32768,    86,
-32768,    38,-32768,    -1,-32768,    16,-32768,-32768,   157,   -28,
    12,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    86,-32768,-32768,   276,
   276,    15,    15,   420,    12,   128,-32768,-32768,-32768,-32768,
-32768,-32768,     4,-32768,   164,-32768,    17,-32768,   164,-32768,
   174,   189,-32768,-32768,   -28,-32768,-32768,   312,   312,    12,
-32768,-32768,-32768,   201,   276,   276,   276,   276,   276,   200,
   180,-32768,    -2,-32768,-32768,   196,-32768,    57,   168,-32768,
-32768,   198,    57,   170,-32768,-32768,   -28,-32768,   209,   213,
-32768,-32768,-32768,   142,   142,   142,-32768,-32768,   190,     1,
-32768,-32768,   205,    40,   204,   177,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   207,   183,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    50,   210,-32768,-32768,
-32768,    20,-32768,-32768,    30,   139,   276,   276,   276,-32768,
-32768,-32768,   276,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   276,   215,   188,   218,   191,-32768,-32768,-32768,-32768,-32768,
   236,   237,-32768
};

static const short yypgoto[] = {-32768,
    91,   -27,   214,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,  -107,     9,-32768,    74,-32768,    46,   -18,
    -6,-32768,   -60,    64,   -22,-32768,    11,     6,    -7,   245
};


#define	YYLAST		517


static const short yytable[] = {    52,
    53,    55,   154,    83,    87,    42,   201,    82,    98,   226,
    41,    45,   144,    58,    26,   145,    49,   174,   202,    57,
    84,   202,   127,    70,    89,    71,   102,   104,   106,   148,
   180,   110,   149,   244,    98,    98,    96,    98,    90,    91,
    98,   146,   203,   246,    72,   203,   175,   188,   111,    73,
    94,    74,    98,    98,   142,   115,    98,   116,   150,   181,
   129,    50,   245,   118,    51,   119,    98,   130,   131,    54,
    75,   134,   247,    68,    98,    69,   117,   208,   228,   219,
   143,    76,   147,    77,   120,   229,    78,   135,   228,   155,
   136,    79,   209,   210,   211,   240,    80,    42,    42,    81,
   151,   152,    97,   133,   192,   193,   194,   195,   196,   137,
   138,   107,    90,    91,   165,   166,   167,   168,   169,   173,
   139,   140,   141,    92,    93,    83,   160,    83,   103,   182,
   187,   184,   161,   162,   163,    42,    42,   176,   105,   179,
   156,   157,    13,   248,   162,    14,    15,    16,    17,   200,
    18,    19,    20,    21,   189,   190,   108,    83,    83,    24,
    25,   214,   215,    42,    42,   137,   138,   205,   206,   133,
   109,   113,   112,   114,   121,   122,   194,   195,   196,   123,
   124,   125,   128,   126,   177,    85,    12,    13,   185,   232,
    14,    15,    16,    17,   235,    18,    19,    20,    21,   237,
   238,   239,   153,   186,    24,    25,   243,   191,   197,   198,
   207,   213,   216,   218,   236,   220,   221,   251,   230,   225,
   231,   233,   242,    83,    83,   202,   234,   254,   255,   257,
   241,   258,   259,   250,   260,   262,   263,   252,   253,   183,
     1,   256,     2,    86,   249,   199,   217,     3,   227,     4,
     5,     6,     7,    56,     0,     8,     0,     9,    10,     0,
    11,    12,    13,     0,     0,    14,    15,    16,    17,     0,
    18,    19,    20,    21,     0,     0,    22,    23,     2,    24,
    25,     0,    26,     3,    27,     4,     5,     6,     7,     0,
     0,     8,     0,     9,    10,     0,    11,    12,    13,     0,
     0,    14,    15,    16,    17,     0,    18,    19,    20,    21,
     0,     0,    98,     0,     2,    24,    25,     0,    26,     3,
    27,     4,     5,     6,     7,     0,     0,     8,     0,     9,
    10,     0,    11,    12,    13,     0,     0,    14,    15,    16,
    17,     0,    18,    19,    20,    21,     0,     0,     0,     0,
     2,    24,    25,     0,    26,     3,    27,     4,     5,     6,
     7,     0,     0,     8,     0,     0,    10,     0,    11,    12,
    13,     0,     0,    14,    15,    16,    17,     0,    18,    19,
    20,    21,     0,     0,     0,     0,     2,    24,    25,     0,
    26,     3,    27,     4,     5,     6,     7,     0,     0,     8,
     0,    95,     0,     0,    11,    12,    13,     0,     0,    14,
    15,    16,    17,     0,    18,    19,    20,    21,     0,     0,
     0,     0,     2,    24,    25,     0,    26,     3,    27,     4,
     5,     6,     7,     0,     0,     8,     0,     0,     0,     0,
    11,    12,    13,     0,     0,    14,    15,    16,    17,     0,
    18,    19,    20,    21,     0,     0,    98,     0,     2,    24,
    25,     0,    26,     3,    27,     4,     5,     6,     7,     0,
     0,     8,     0,     0,     0,     0,    11,    12,    13,     0,
     0,    14,    15,    16,    17,     0,    18,    19,    20,    21,
     0,     0,     0,     0,     0,    24,    25,     0,    26,     0,
    27,    59,    60,    61,    62,     0,    63,     0,    64,    65,
     0,     0,     0,     0,     0,    66,    67
};

static const short yycheck[] = {     6,
     7,     9,   110,    26,    32,     0,     9,    26,    37,     9,
     0,    37,    14,    45,    43,    17,    21,    14,    21,    20,
    27,    21,    83,    21,    37,    23,    49,    50,    51,    14,
    14,    54,    17,    14,    37,    37,    44,    37,    24,    25,
    37,    43,    45,    14,    42,    45,    43,   155,    56,    21,
    39,    23,    37,    37,    17,    21,    37,    23,    43,    43,
    88,    21,    43,    21,    21,    23,    37,    90,    91,    21,
    42,    94,    43,    21,    37,    21,    42,    21,    39,   187,
   103,    21,   105,    21,    42,    46,    21,    95,    39,   112,
    97,    21,    36,    37,    38,    46,    21,    92,    93,    21,
   107,   108,     4,    93,   165,   166,   167,   168,   169,    24,
    25,    14,    24,    25,   137,   138,   139,   140,   141,   142,
    35,    36,    37,    35,    36,   148,   134,   150,    36,   148,
   153,   150,     5,     6,     7,   130,   131,   144,    36,   146,
   130,   131,    23,     5,     6,    26,    27,    28,    29,   172,
    31,    32,    33,    34,   161,   162,    14,   180,   181,    40,
    41,   180,   181,   158,   159,    24,    25,   174,   175,   159,
    45,    21,    46,    21,    21,    21,   237,   238,   239,    21,
    21,    21,    46,    44,    21,    21,    22,    23,    15,   212,
    26,    27,    28,    29,   217,    31,    32,    33,    34,   222,
   223,   224,    46,    15,    40,    41,   229,     7,     9,    30,
    15,    44,    15,    44,   221,     7,     4,   240,    15,    30,
    44,    15,   229,   246,   247,    21,    44,   246,   247,    15,
    21,    44,    15,   240,    44,     0,     0,   244,   245,   149,
     1,   248,     3,    30,   236,   172,   183,     8,   203,    10,
    11,    12,    13,     9,    -1,    16,    -1,    18,    19,    -1,
    21,    22,    23,    -1,    -1,    26,    27,    28,    29,    -1,
    31,    32,    33,    34,    -1,    -1,    37,    38,     3,    40,
    41,    -1,    43,     8,    45,    10,    11,    12,    13,    -1,
    -1,    16,    -1,    18,    19,    -1,    21,    22,    23,    -1,
    -1,    26,    27,    28,    29,    -1,    31,    32,    33,    34,
    -1,    -1,    37,    -1,     3,    40,    41,    -1,    43,     8,
    45,    10,    11,    12,    13,    -1,    -1,    16,    -1,    18,
    19,    -1,    21,    22,    23,    -1,    -1,    26,    27,    28,
    29,    -1,    31,    32,    33,    34,    -1,    -1,    -1,    -1,
     3,    40,    41,    -1,    43,     8,    45,    10,    11,    12,
    13,    -1,    -1,    16,    -1,    -1,    19,    -1,    21,    22,
    23,    -1,    -1,    26,    27,    28,    29,    -1,    31,    32,
    33,    34,    -1,    -1,    -1,    -1,     3,    40,    41,    -1,
    43,     8,    45,    10,    11,    12,    13,    -1,    -1,    16,
    -1,    18,    -1,    -1,    21,    22,    23,    -1,    -1,    26,
    27,    28,    29,    -1,    31,    32,    33,    34,    -1,    -1,
    -1,    -1,     3,    40,    41,    -1,    43,     8,    45,    10,
    11,    12,    13,    -1,    -1,    16,    -1,    -1,    -1,    -1,
    21,    22,    23,    -1,    -1,    26,    27,    28,    29,    -1,
    31,    32,    33,    34,    -1,    -1,    37,    -1,     3,    40,
    41,    -1,    43,     8,    45,    10,    11,    12,    13,    -1,
    -1,    16,    -1,    -1,    -1,    -1,    21,    22,    23,    -1,
    -1,    26,    27,    28,    29,    -1,    31,    32,    33,    34,
    -1,    -1,    -1,    -1,    -1,    40,    41,    -1,    43,    -1,
    45,    26,    27,    28,    29,    -1,    31,    -1,    33,    34,
    -1,    -1,    -1,    -1,    -1,    40,    41
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/local/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/local/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
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
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
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

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 215 "./parse.y"
{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = yyvsp[-1].command;
			  eof_encountered = 0;
			  discard_parser_constructs (0);
			  YYACCEPT;
			;
    break;}
case 2:
#line 224 "./parse.y"
{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			;
    break;}
case 3:
#line 231 "./parse.y"
{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  discard_parser_constructs (1);
			  if (interactive)
			    {
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			;
    break;}
case 4:
#line 246 "./parse.y"
{
			  /* Case of EOF seen by itself.  Do ignoreeof or
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			;
    break;}
case 5:
#line 256 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); ;
    break;}
case 6:
#line 258 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-1].word_list); ;
    break;}
case 7:
#line 262 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_direction, redir);
			;
    break;}
case 8:
#line 267 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_direction, redir);
			;
    break;}
case 9:
#line 272 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_direction, redir);
			;
    break;}
case 10:
#line 277 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_direction, redir);
			;
    break;}
case 11:
#line 282 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_appending_to, redir);
			;
    break;}
case 12:
#line 287 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_appending_to, redir);
			;
    break;}
case 13:
#line 292 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 14:
#line 298 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 15:
#line 304 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (0, r_duplicating_input, redir);
			;
    break;}
case 16:
#line 309 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input, redir);
			;
    break;}
case 17:
#line 314 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (1, r_duplicating_output, redir);
			;
    break;}
case 18:
#line 319 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output, redir);
			;
    break;}
case 19:
#line 324 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_duplicating_input_word, redir);
			;
    break;}
case 20:
#line 329 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input_word, redir);
			;
    break;}
case 21:
#line 334 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_duplicating_output_word, redir);
			;
    break;}
case 22:
#line 339 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output_word, redir);
			;
    break;}
case 23:
#line 344 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (0, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 24:
#line 351 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 25:
#line 358 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (1, r_close_this, redir);
			;
    break;}
case 26:
#line 363 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			;
    break;}
case 27:
#line 368 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (0, r_close_this, redir);
			;
    break;}
case 28:
#line 373 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			;
    break;}
case 29:
#line 378 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_err_and_out, redir);
			;
    break;}
case 30:
#line 383 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_output, redir);
			;
    break;}
case 31:
#line 388 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_output, redir);
			;
    break;}
case 32:
#line 393 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_force, redir);
			;
    break;}
case 33:
#line 398 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_force, redir);
			;
    break;}
case 34:
#line 405 "./parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; ;
    break;}
case 35:
#line 407 "./parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; ;
    break;}
case 36:
#line 409 "./parse.y"
{ yyval.element.redirect = yyvsp[0].redirect; yyval.element.word = 0; ;
    break;}
case 37:
#line 413 "./parse.y"
{
			  yyval.redirect = yyvsp[0].redirect;
			;
    break;}
case 38:
#line 417 "./parse.y"
{
			  register REDIRECT *t;

			  for (t = yyvsp[-1].redirect; t->next; t = t->next)
			    ;
			  t->next = yyvsp[0].redirect;
			  yyval.redirect = yyvsp[-1].redirect;
			;
    break;}
case 39:
#line 428 "./parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, (COMMAND *)NULL); ;
    break;}
case 40:
#line 430 "./parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, yyvsp[-1].command); ;
    break;}
case 41:
#line 434 "./parse.y"
{ yyval.command = clean_simple_command (yyvsp[0].command); ;
    break;}
case 42:
#line 436 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 43:
#line 438 "./parse.y"
{
			  COMMAND *tc;

			  tc = yyvsp[-1].command;
			  /* According to Posix.2 3.9.5, redirections
			     specified after the body of a function should
			     be attached to the function and performed when
			     the function is executed, not as part of the
			     function definition command. */
			  if (tc->type == cm_function_def)
			    {
			      tc = tc->value.Function_def->command;
			      if (tc->type == cm_group)
			        tc = tc->value.Group->command;
			    }
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
			;
    break;}
case 44:
#line 467 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 45:
#line 469 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 46:
#line 471 "./parse.y"
{ yyval.command = make_while_command (yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 47:
#line 473 "./parse.y"
{ yyval.command = make_until_command (yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 48:
#line 475 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 49:
#line 477 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 50:
#line 479 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 51:
#line 481 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 52:
#line 483 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 53:
#line 487 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 54:
#line 489 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 55:
#line 491 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 56:
#line 493 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 57:
#line 495 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); ;
    break;}
case 58:
#line 497 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); ;
    break;}
case 59:
#line 501 "./parse.y"
{
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			;
    break;}
case 60:
#line 505 "./parse.y"
{
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command);
			;
    break;}
case 61:
#line 509 "./parse.y"
{
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			;
    break;}
case 62:
#line 513 "./parse.y"
{
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			;
    break;}
case 63:
#line 517 "./parse.y"
{
			  yyval.command = make_select_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command);
			;
    break;}
case 64:
#line 521 "./parse.y"
{
			  yyval.command = make_select_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command);
			;
    break;}
case 65:
#line 527 "./parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, (PATTERN_LIST *)NULL); ;
    break;}
case 66:
#line 529 "./parse.y"
{ yyval.command = make_case_command (yyvsp[-5].word, yyvsp[-2].pattern); ;
    break;}
case 67:
#line 531 "./parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, yyvsp[-1].pattern); ;
    break;}
case 68:
#line 535 "./parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command, function_dstart, function_bstart); ;
    break;}
case 69:
#line 539 "./parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command, function_dstart, function_bstart); ;
    break;}
case 70:
#line 542 "./parse.y"
{ yyval.command = make_function_def (yyvsp[-2].word, yyvsp[0].command, function_dstart, function_bstart); ;
    break;}
case 71:
#line 546 "./parse.y"
{ yyvsp[-1].command->flags |= CMD_WANT_SUBSHELL; yyval.command = yyvsp[-1].command; ;
    break;}
case 72:
#line 550 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, (COMMAND *)NULL); ;
    break;}
case 73:
#line 552 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-5].command, yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 74:
#line 554 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[-1].command); ;
    break;}
case 75:
#line 559 "./parse.y"
{ yyval.command = make_group_command (yyvsp[-1].command); ;
    break;}
case 76:
#line 563 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-2].command, yyvsp[0].command, (COMMAND *)NULL); ;
    break;}
case 77:
#line 565 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[0].command); ;
    break;}
case 78:
#line 567 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, yyvsp[0].command); ;
    break;}
case 80:
#line 572 "./parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; ;
    break;}
case 81:
#line 576 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); ;
    break;}
case 82:
#line 578 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); ;
    break;}
case 83:
#line 580 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); ;
    break;}
case 84:
#line 582 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); ;
    break;}
case 86:
#line 587 "./parse.y"
{ yyvsp[-1].pattern->next = yyvsp[-2].pattern; yyval.pattern = yyvsp[-1].pattern; ;
    break;}
case 87:
#line 591 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); ;
    break;}
case 88:
#line 593 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-2].word_list); ;
    break;}
case 89:
#line 602 "./parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			 ;
    break;}
case 91:
#line 611 "./parse.y"
{
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 93:
#line 618 "./parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, (COMMAND *)NULL, '&');
			;
    break;}
case 95:
#line 629 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); ;
    break;}
case 96:
#line 631 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); ;
    break;}
case 97:
#line 633 "./parse.y"
{
			  if (yyvsp[-3].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-3].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '&');
			;
    break;}
case 98:
#line 640 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); ;
    break;}
case 99:
#line 642 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); ;
    break;}
case 100:
#line 644 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 106:
#line 663 "./parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			;
    break;}
case 107:
#line 669 "./parse.y"
{
			  if (yyvsp[-1].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents ();
			;
    break;}
case 108:
#line 678 "./parse.y"
{
			  yyval.command = yyvsp[-1].command;
			  if (need_here_doc)
			    gather_here_documents ();
			;
    break;}
case 109:
#line 686 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); ;
    break;}
case 110:
#line 688 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); ;
    break;}
case 111:
#line 690 "./parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, '&');
			;
    break;}
case 112:
#line 697 "./parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, ';'); ;
    break;}
case 113:
#line 700 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 114:
#line 704 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 115:
#line 706 "./parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 116:
#line 711 "./parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-1].number;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 117:
#line 716 "./parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-2].number;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 118:
#line 721 "./parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-1].number|CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 119:
#line 729 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '|'); ;
    break;}
case 120:
#line 731 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 121:
#line 735 "./parse.y"
{ yyval.number = CMD_TIME_PIPELINE; ;
    break;}
case 122:
#line 737 "./parse.y"
{ yyval.number = CMD_TIME_PIPELINE|CMD_TIME_POSIX; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/local/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

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

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 739 "./parse.y"


/* Possible states for the parser that require it to do special things. */
#define PST_CASEPAT	0x001		/* in a case pattern list */
#define PST_ALEXPNEXT	0x002		/* expand next word for aliases */
#define PST_ALLOWOPNBRC	0x004		/* allow open brace for function def */
#define PST_NEEDCLOSBRC	0x008		/* need close brace */
#define PST_DBLPAREN	0x010		/* double-paren parsing */
#define PST_SUBSHELL	0x020		/* ( ... ) subshell */
#define PST_CMDSUBST	0x040		/* $( ... ) command substitution */
#define PST_CASESTMT	0x080		/* parsing a case statement */

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_INITIAL_SIZE 496
#define TOKEN_DEFAULT_GROW_SIZE 512

/* Shell meta-characters that, when unquoted, separate words. */
#define shellmeta(c)	(strchr ("()<>;&|", (c)) != 0)
#define shellbreak(c)	(strchr ("()<>;&| \t\n", (c)) != 0)
#define shellquote(c)	((c) == '"' || (c) == '`' || (c) == '\'')
#define shellexp(c)	((c) == '$' || (c) == '<' || (c) == '>')

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

void
debug_parser (i)
     int i;
{
#if YYDEBUG != 0
  yydebug = i;
#endif
}

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
  bash_input.getter = (Function *)NULL;
  bash_input.ungetter = (Function *)NULL;
}

/* Set the contents of the current bash input stream from
   GET, UNGET, TYPE, NAME, and LOCATION. */
void
init_yy_io (get, unget, type, name, location)
     Function *get, *unget;
     enum stream_type type;
     char *name;
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

/* Call this to get the next character of input. */
int
yy_getc ()
{
  return (*(bash_input.getter)) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
int
yy_ungetc (c)
     int c;
{
  return (*(bash_input.ungetter)) (c);
}

#if defined (BUFFERED_INPUT)
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
  int line_len, c;

  if (!current_readline_line)
    {
      if (!bash_readline_initialized)
	initialize_readline ();

#if defined (JOB_CONTROL)
      if (job_control)
	give_terminal_to (shell_pgrp);
#endif /* JOB_CONTROL */

      if (signal_is_ignored (SIGINT) == 0)
	{
	  old_sigint = (SigHandler *)set_signal_handler (SIGINT, sigint_sighandler);
	  interrupt_immediately++;
	}

      current_readline_line = readline (current_readline_prompt ?
      					  current_readline_prompt : "");

      if (signal_is_ignored (SIGINT) == 0)
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

      current_readline_line = xrealloc (current_readline_line, 2 + line_len);
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
      c = (unsigned char)current_readline_line[current_readline_line_index++];
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
  register int c;

  string = bash_input.location.string;
  c = EOF;

  /* If the string doesn't exist, or is empty, EOF found. */
  if (string && *string)
    {
      c = *(unsigned char *)string++;
      bash_input.location.string = string;
    }
  return (c);
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
     char *string, *name;
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

static int
yy_stream_get ()
{
  int result = EOF;

  if (bash_input.location.file)
    {
#if !defined (HAVE_RESTARTABLE_SYSCALLS)
      result = getc_with_restart (bash_input.location.file);
#else /* HAVE_RESTARTABLE_SYSCALLS */
      result = getc (bash_input.location.file);
      result = (feof (bash_input.location.file)) ? EOF : (unsigned char)result;
#endif /* HAVE_RESTARTABLE_SYSCALLS */
    }
  return (result);
}

static int
yy_stream_unget (c)
     int c;
{
#if !defined (HAVE_RESTARTABLE_SYSCALLS)
  return (ungetc_with_restart (c, bash_input.location.file));
#else /* HAVE_RESTARTABLE_SYSCALLS */
  return (ungetc (c, bash_input.location.file));
#endif /* HAVE_RESTARTABLE_SYSCALLS */
}

void
with_input_from_stream (stream, name)
     FILE *stream;
     char *name;
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
    {
      saver->bstream = buffers[bash_input.location.buffered_fd];
      buffers[bash_input.location.buffered_fd] = (BUFFERED_STREAM *)NULL;
    }
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
		}
	    }
	  buffers[bash_input.location.buffered_fd] = saver->bstream;
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

/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list' is:
 *
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */

#if defined (ALIAS)

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
  alias_t *expander;   /* alias that caused this line to be pushed. */
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
  STRING_SAVER *temp = (STRING_SAVER *) xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
  temp->expander = ap;
  temp->next = pushed_string_list;
  pushed_string_list = temp;

  if (ap)
    ap->flags |= AL_BEINGEXPANDED;

  shell_input_line = s;
  shell_input_line_size = strlen (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
  parser_state &= ~PST_ALEXPNEXT;
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

  if (t->expander)
    t->expander->flags &= ~AL_BEINGEXPANDED;

  free ((char *)t);
}

static void
free_string_list ()
{
  register STRING_SAVER *t, *t1;

  for (t = pushed_string_list; t; )
    {
      t1 = t->next;
      FREE (t->saved_line);
      t->expander->flags &= ~AL_BEINGEXPANDED;
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

#endif /* ALIAS */

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

      if (c == 0)
	continue;

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
  { (char *)NULL, 0}
};

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
	ds.delimiters = xrealloc \
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

static int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  register int i;
  int c;
  static int mustpop = 0;

  QUIT;

#if defined (ALIAS)
  /* If shell_input_line[shell_input_line_index] == 0, but there is
     something on the pushed list of strings, then we don't want to go
     off and get another line.  We let the code down below handle it. */

  if (!shell_input_line || ((!shell_input_line[shell_input_line_index]) &&
			    (pushed_string_list == (STRING_SAVER *)NULL)))
#else /* !ALIAS */
  if (!shell_input_line || !shell_input_line[shell_input_line_index])
#endif /* !ALIAS */
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

      while (c = yy_getc ())
	{
	  /* Allow immediate exit if interrupted during input. */
	  QUIT;

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
	    }
	}
      /* XXX - this is grotesque */
      else if (remember_on_history && shell_input_line &&
	       shell_input_line[0] == '\0' &&
	       current_command_line_count > 1 && current_delimiter (dstack))
	{
	  /* We know shell_input_line[0] == 0 and we're reading some sort of
	     quoted string.  This means we've got a line consisting of only
	     a newline in a quoted string.  We want to make sure this line
	     gets added to the history. */
	  maybe_add_history (shell_input_line);
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
	    shell_input_line = xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  shell_input_line[shell_input_line_len] = '\n';
	  shell_input_line[shell_input_line_len + 1] = '\0';
	}
    }

  c = shell_input_line[shell_input_line_index];

  if (c)
    shell_input_line_index++;

  if (c == '\\' && remove_quoted_newline &&
      shell_input_line[shell_input_line_index] == '\n')
    {
	prompt_again ();
	line_number++;
	goto restart_read;
    }

#if defined (ALIAS)
  /* If C is NULL, we have reached the end of the current input string.  If
     pushed_string_list is non-empty, it's time to pop to the previous string
     because we have fully consumed the result of the last alias expansion.
     Do it transparently; just return the next character of the string popped
     to. */
  if (!c && (pushed_string_list != (STRING_SAVER *)NULL))
    {
      if (mustpop)
        {
          pop_string ();
          c = shell_input_line[shell_input_line_index];
	  if (c)
	    shell_input_line_index++;
	  mustpop--;
        }
      else
        {
          mustpop++;
          c = ' ';
        }
    }
#endif /* ALIAS */

  if (!c && shell_input_line_terminator == EOF)
    return ((shell_input_line_index != 0) ? '\n' : EOF);

  return ((unsigned char)c);
}

/* Put C back into the input for the shell. */
static void
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
}

static void
shell_ungetchar ()
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line_index--;
}

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
  Function *temp_last, *temp_this;
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
	      if (word_token_alist[i].token == ESAC) \
		parser_state &= ~(PST_CASEPAT|PST_CASESTMT); \
	      else if (word_token_alist[i].token == CASE) \
		parser_state |= PST_CASESTMT; \
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

       It is eligible for expansion if the shell is in interactive mode, and
       the token is unquoted and the last token read was a command
       separator (or expand_next_token is set), and we are currently
       processing an alias (pushed_string_list is non-empty) and this
       token is not the same as the current or any previously
       processed alias.

       Special cases that disqualify:
	 In a pattern list in a case statement (parser_state & PST_CASEPAT). */
static int
alias_expand_token (token)
     char *token;
{
  char *expanded;
  alias_t *ap;

  if (((parser_state & PST_ALEXPNEXT) || command_token_position (last_read_token)) &&
	(parser_state & PST_CASEPAT) == 0)
    {
      ap = find_alias (token);

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

/* Handle special cases of token recognition:
	IN is recognized if the last token was WORD and the token
	before that was FOR or CASE or SELECT.

	DO is recognized if the last token was WORD and the token
	before that was FOR or SELECT.

	ESAC is recognized if the last token caused `esacs_needed_count'
	to be set

	`{' is recognized if the last token as WORD and the token
	before that was FUNCTION.

	`}' is recognized if there is an unclosed `{' prsent.
*/

static int
special_case_tokens (token)
     char *token;
{
  if ((last_read_token == WORD) &&
#if defined (SELECT_COMMAND)
      ((token_before_that == FOR) || (token_before_that == CASE) || (token_before_that == SELECT)) &&
#else
      ((token_before_that == FOR) || (token_before_that == CASE)) &&
#endif
      (token[0] == 'i' && token[1] == 'n' && token[2] == 0))
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
      (token[0] == 'd' && token[1] == 'o' && token[2] == '\0'))
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
      if (STREQ (token, "esac"))
	{
	  parser_state &= ~PST_CASEPAT;
	  return (ESAC);
	}
    }

  /* The start of a shell function definition. */
  if (parser_state & PST_ALLOWOPNBRC)
    {
      parser_state &= ~PST_ALLOWOPNBRC;
      if (token[0] == '{' && token[1] == '\0')		/* } */
	{
	  open_brace_count++;
	  function_bstart = line_number;
	  return ('{');					/* } */
	}
    }

  if (open_brace_count && reserved_word_acceptable (last_read_token) && token[0] == '}' && !token[1])
    {
      open_brace_count--;		/* { */
      return ('}');
    }

  /* Handle -p after `time'. */
  if (last_read_token == TIME && token[0] == '-' && token[1] == 'p' && !token[2])
    return (TIMEOPT);

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

#if defined (ALIAS)
  if (pushed_string_list)
    {
      free_string_list ();
      pushed_string_list = (STRING_SAVER *)NULL;
    }
#endif /* ALIAS */

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

  if (character == '#' && (!interactive || interactive_comments))
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
  if (shellmeta (character) && ((parser_state & PST_DBLPAREN) == 0))
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

#if defined (DPAREN_ARITHMETIC)
	    case '(':		/* ) */
	      if (reserved_word_acceptable (last_read_token))
		{
		  int cmdtyp, sline;
		  char *wval;

		  sline = line_number;
		  cmdtyp = parse_arith_cmd (&wval);
		  if (cmdtyp == 1)	/* arithmetic command */
		    {
		      word_desc_to_read = make_word (wval);
		      word_desc_to_read->flags = W_QUOTED;
		      token_to_read = WORD;
		      free (wval);
		      yylval.word = make_word ("let");
		      return (WORD);
		    }
		  else if (cmdtyp == 0)	/* nested subshell */
		    {
		      push_string (wval, 0, (alias_t *)NULL);
		      if ((parser_state & PST_CASEPAT) == 0)
			parser_state |= PST_SUBSHELL;
		      return (character);
		    }
		  else			/* ERROR */
		    return -1;
		}
	      break;
#endif
	    }
	}
      else if (character == '<' && peek_char == '&')
	return (LESS_AND);
      else if (character == '>' && peek_char == '&')
	return (GREATER_AND);
      else if (character == '<' && peek_char == '>')
	return (LESS_GREATER);
      else if (character == '>' && peek_char == '|')
	return (GREATER_BAR);
      else if (peek_char == '>' && character == '&')
	return (AND_GREATER);

      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if (character == ')' && last_read_token == '(' && token_before_that == WORD)
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
      if (character == '(' && (parser_state & PST_CASEPAT) == 0) /* ) */
	parser_state |= PST_SUBSHELL;
      /*(*/
      else if ((parser_state & PST_CASEPAT) && character == ')')
        parser_state &= ~PST_CASEPAT;
      /*(*/
      else if ((parser_state & PST_SUBSHELL) && character == ')')
	parser_state &= ~PST_SUBSHELL;

#if defined (PROCESS_SUBSTITUTION)
      /* Check for the constructs which introduce process substitution.
	 Shells running in `posix mode' don't do process substitution. */
      if (posixly_correct ||
	  ((character != '>' && character != '<') || peek_char != '('))
#endif /* PROCESS_SUBSTITUTION */
	return (character);
    }

  /* Hack <&- (close stdin) case. */
  if (character == '-' && (last_read_token == LESS_AND || last_read_token == GREATER_AND))
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

/* Match a $(...) or other grouping construct.  This has to handle embedded
   quoted strings ('', ``, "") and nested constructs.  It also must handle
   reprompting the user, if necessary, after reading a newline, and returning
   correct error values if it reads EOF. */
static char matched_pair_error;
static char *
parse_matched_pair (qc, open, close, lenp, flags)
     int qc;	/* `"' if this construct is within double quotes */
     int open, close;
     int *lenp, flags;
{
  int count, ch, was_dollar;
  int pass_next_character, nestlen, start_lineno;
  char *ret, *nestret;
  int retind, retsize;

  count = 1;
  pass_next_character = was_dollar = 0;

  ret = xmalloc (retsize = 64);
  retind = 0;

  start_lineno = line_number;
  while (count)
    {
      ch = shell_getc (qc != '\'' && pass_next_character == 0);
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
	  if (ch == CTLESC || ch == CTLNUL)
	    ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if (ch == CTLESC || ch == CTLNUL)	/* special shell escapes */
	{
	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if (ch == close)		/* ending delimiter */
	count--;
      else if (ch == open)		/* nested begin */
	count++;

      /* Add this character. */
      RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
      ret[retind++] = ch;

      if (open == '\'')			/* '' inside grouping construct */
	continue;

      if (ch == '\\')			/* backslashes */
	pass_next_character++;

      if (open != close)		/* a grouping construct */
	{
	  if (shellquote (ch))
	    {
	      /* '', ``, or "" inside $(...) or other grouping construct. */
	      push_delimiter (dstack, ch);
	      nestret = parse_matched_pair (ch, ch, ch, &nestlen, 0);
	      pop_delimiter (dstack);
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
	}
      /* Parse an old-style command substitution within double quotes as a
	 single word. */
      /* XXX - sh and ksh93 don't do this - XXX */
      else if (open == '"' && ch == '`')
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
      else if (was_dollar && (ch == '(' || ch == '{' || ch == '['))	/* ) } ] */
	/* check for $(), $[], or ${} inside quoted string. */
	{
	  if (open == ch)	/* undo previous increment */
	    count--;
	  if (ch == '(')		/* ) */
	    nestret = parse_matched_pair (0, '(', ')', &nestlen, 0);
	  else if (ch == '{')		/* } */
	    nestret = parse_matched_pair (0, '{', '}', &nestlen, 0);
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
      was_dollar = (ch == '$');
    }

  ret[retind] = '\0';
  if (lenp)
    *lenp = retind;
  return ret;
}

#if defined (DPAREN_ARITHMETIC)
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
  char *ttok, *token;
  int ttoklen;

  exp_lineno = line_number;
  ttok = parse_matched_pair (0, '(', ')', &ttoklen, 0);
  rval = 1;
  if (ttok == &matched_pair_error)
    return -1;
  /* Check that the next character is the closing right paren.  If
     not, this is a syntax error. ( */
  if ((c = shell_getc (0)) != ')')
    rval = 0;

  token = xmalloc(ttoklen + 4);

  /* (( ... )) -> "..." */
  token[0] = (rval == 1) ? '"' : '(';
  strncpy (token + 1, ttok, ttoklen - 1);	/* don't copy the final `)' */
  if (rval == 1)
    {
      token[ttoklen] = '"';
      token[ttoklen+1] = '\0';
    }
  else
    {
      token[ttoklen] = ')';
      token[ttoklen+1] = c;
      token[ttoklen+2] = '\0';
    }
  *ep = token;
  FREE (ttok);
  return rval;
}
#endif /* DPAREN_ARITHMETIC */

static int
read_token_word (character)
     int character;
{
  /* The value for YYLVAL when a WORD is read. */
  WORD_DESC *the_word;

  /* Index into the token that we are building. */
  int token_index;

  /* ALL_DIGITS becomes zero when we see a non-digit. */
  int all_digits;

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

  if (token_buffer_size < TOKEN_DEFAULT_INITIAL_SIZE)
    token = xrealloc (token, token_buffer_size = TOKEN_DEFAULT_INITIAL_SIZE);

  token_index = 0;
  all_digits = digit (character);
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
      if (character == '\\')
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
		  (cd == '"' && member (peek_char, slashify_in_quotes)))
		pass_next_character++;

	      quoted = 1;
	      goto got_character;
	    }
	}

      /* Parse a matched pair of quote characters. */
      if (shellquote (character))
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
	  all_digits = 0;
	  quoted = 1;
	  dollar_present |= (character == '"' && strchr (ttok, '$') != 0);
	  FREE (ttok);
	  goto next_character;
	}

      /* If the delimiter character is not single quote, parse some of
	 the shell expansions that must be read as a single word. */
#if defined (PROCESS_SUBSTITUTION)
      if (character == '$' || character == '<' || character == '>')
#else
      if (character == '$')
#endif /* !PROCESS_SUBSTITUTION */
	{
	  peek_char = shell_getc (1);
	  /* $(...), <(...), >(...), $((...)), ${...}, and $[...] constructs */
	  if (peek_char == '(' ||
		((peek_char == '{' || peek_char == '[') && character == '$'))	/* ) ] } */
	    {
	      if (peek_char == '{')		/* } */
	        ttok = parse_matched_pair (cd, '{', '}', &ttoklen, 0);
	      else if (peek_char == '(')		/* ) */
		{
		  /* XXX - push and pop the `(' as a delimiter for use by
		     the command-oriented-history code.  This way newlines
		     appearing in the $(...) string get added to the
		     history literally rather than causing a possibly-
		     incorrect `;' to be added. */
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
	      all_digits = 0;
	      goto next_character;
	    }
	  /* This handles $'...' and $"..." new-style quoted strings. */
	  else if (character == '$' && (peek_char == '\'' || peek_char == '"'))
	    {
	      ttok = parse_matched_pair (peek_char, peek_char, peek_char, &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;
	      if (peek_char == '\'')
		ttrans = ansiexpand (ttok, 0, ttoklen - 1, &ttranslen);
	      else
		ttrans = localeexpand (ttok, 0, ttoklen - 1, &ttranslen);
	      free (ttok);
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttranslen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttrans);
	      token_index += ttranslen;
	      token[token_index++] = peek_char;
	      FREE (ttrans);
	      quoted = 1;
	      all_digits = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}

#if defined (ARRAY_VARS)
      /* Identify possible compound array variable assignment. */
      else if (character == '=')
	{
	  peek_char = shell_getc (1);
	  if (peek_char == '(')		/* ) */
	    {
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
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
	      all_digits = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif

      /* When not parsing a multi-character word construct, shell meta-
	 characters break words. */
      if (shellbreak (character))
	{
	  shell_ungetc (character);
	  goto got_token;
	}

    got_character:

      all_digits &= digit (character);
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
  if (all_digits && (character == '<' || character == '>' ||
		    last_read_token == LESS_AND ||
		    last_read_token == GREATER_AND))
      {
	yylval.number = atoi (token);
	return (NUMBER);
      }

  /* Check for special case tokens. */
  result = special_case_tokens (token);
  if (result >= 0)
    return result;

#if defined (ALIAS)
  /* Posix.2 does not allow reserved words to be aliased, so check for all
     of them, including special cases, before expanding the current token
     as an alias. */
  if (posixly_correct)
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
  if (posixly_correct == 0)
#endif
    CHECK_FOR_RESERVED_WORD (token);

  the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
  the_word->word = xmalloc (1 + token_index);
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

/* $'...' ANSI-C expand the portion of STRING between START and END and
   return the result.  The result cannot be longer than the input string. */
static char *
ansiexpand (string, start, end, lenp)
     char *string;
     int start, end, *lenp;
{
  char *temp, *t;
  int len, tlen;

  temp = xmalloc (end - start + 1);
  for (tlen = 0, len = start; len < end; )
    temp[tlen++] = string[len++];
  temp[tlen] = '\0';

  if (*temp)
    {
      t = ansicstr (temp, tlen, (int *)NULL, lenp);
      free (temp);
      return (t);
    }
  else
    {
      if (lenp)
	*lenp = 0;
      return (temp);
    }
}

/* $"..." -- Translate the portion of STRING between START and END
   according to current locale using gettext (if available) and return
   the result.  The caller will take care of leaving the quotes intact.
   The string will be left without the leading `$' by the caller.
   If translation is performed, the translated string will be double-quoted
   by the caller.  The length of the translated string is returned in LENP,
   if non-null. */
static char *
localeexpand (string, start, end, lenp)
     char *string;
     int start, end, *lenp;
{
  int len, tlen;
  char *temp, *t;

  temp = xmalloc (end - start + 1);
  for (tlen = 0, len = start; len < end; )
    temp[tlen++] = string[len++];
  temp[tlen] = '\0';

  /* If we're just dumping translatable strings, don't do anything. */
  if (dump_translatable_strings)
    {
      printf ("\"%s\"\n", temp);
      if (lenp)
	*lenp = tlen;
      return (temp);
    }
  else if (*temp)
    {
      t = localetrans (temp, tlen, &len);
      free (temp);
      if (lenp)
	*lenp = len;
      return (t);
    }
  else
    {
      if (lenp)
	*lenp = 0;
      return (temp);
    }
}

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (token)
     int token;
{
  if (token == '\n' || token == ';' || token == '(' || token == ')' ||
      token == '|' || token == '&' || token == '{' ||
      token == '}' ||			/* XXX */
      token == AND_AND ||
      token == BANG ||
      token == TIME || token == TIMEOPT ||
      token == DO ||
      token == ELIF ||
      token == ELSE ||
      token == FI ||
      token == IF ||
      token == OR_OR ||
      token == SEMI_SEMI ||
      token == THEN ||
      token == UNTIL ||
      token == WHILE ||
      token == DONE ||		/* XXX these two are experimental */
      token == ESAC ||
      token == 0)
    return (1);
  else
    return (0);
}

/* Return the index of TOKEN in the alist of reserved words, or -1 if
   TOKEN is not a shell reserved word. */
int
find_reserved_word (token)
     char *token;
{
  int i;
  for (i = 0; word_token_alist[i].word; i++)
    if (STREQ (token, word_token_alist[i].word))
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
	  temp_prompt = xmalloc (1);
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
      temp_prompt = xmalloc (1);
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

static void
print_prompt ()
{
  fprintf (stderr, "%s", current_decoded_prompt);
  fflush (stderr);
}

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:

	\a	bell (ascii 07)
	\e	escape (ascii 033)
	\d	the date in Day Mon Date format
	\h	the hostname up to the first `.'
	\H	the hostname
	\n	CRLF
	\s	the name of the shell
	\t	the time in 24-hour hh:mm:ss format
	\T	the time in 12-hour hh:mm:ss format
	\@	the time in 12-hour am/pm format
	\v	the version of bash (e.g., 2.00)
	\V	the release of bash, version + patchlevel (e.g., 2.00.0)
	\w	the current working directory
	\W	the last element of $PWD
	\u	your username
	\#	the command number of this command
	\!	the history number of this command
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
#if defined (PROMPT_STRING_DECODE)
  int result_size, result_index;
  int c, n;
  char *temp, octal_string[4];
  time_t the_time;

  result = xmalloc (result_size = PROMPT_GROWTH);
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
	      temp = xmalloc (3);

	      if (n == CTLESC || n == CTLNUL)
		{
		  string += 3;
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
		  string += 3;
		  temp[0] = n;
		  temp[1] = '\0';
		}

	      c = 0;
	      goto add_string;

	    case 't':
	    case 'd':
	    case 'T':
	    case '@':
	      /* Make the current time/date into a string. */
	      the_time = time (0);
	      temp = ctime (&the_time);

	      temp = (c != 'd') ? savestring (temp + 11) : savestring (temp);
	      temp[(c != 'd') ? 8 : 10] = '\0';

	      /* quick and dirty conversion to 12-hour time */
	      if (c == 'T' || c == '@')
		{
		  if (c == '@')
		    {
		      temp[5] = 'a';	/* am/pm format */
		      temp[6] = 'm';
		      temp[7] = '\0';
		    }
		  c = temp[2];
		  temp[2] = '\0';
		  n = atoi (temp);
		  temp[2] = c;
		  n -= 12;
		  if (n > 0)
		    {
		      temp[0] = (n / 10) + '0';
		      temp[1] = (n % 10) + '0';
		    }
		  if (n >= 0 && temp[5] == 'a')
		    temp[5] = 'p';
		}
	      goto add_string;

	    case 'n':
	      temp = xmalloc (3);
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
	      temp = xmalloc (8);
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

		temp = get_string_value ("PWD");

		if (temp == 0)
		  {
		    if (getcwd (t_string, sizeof(t_string)) == 0)
		      {
		        t_string[0] = '.';
		        t_string[1] = '\0';
		      }
		  }
		else
		  strcpy (t_string, temp);

		if (c == 'W')
		  {
		    t = strrchr (t_string, '/');
		    if (t && t != t_string)
		      strcpy (t_string, t + 1);
		  }
		else
		  strcpy (t_string, polite_directory_format (t_string));

		/* If we're going to be expanding the prompt string later,
		   quote the directory name. */
		if (promptvars || posixly_correct)
		  temp = backslash_quote (t_string);
		else
		  temp = savestring (t_string);

		goto add_string;
	      }

	    case 'u':
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
	      temp = xmalloc (2);
	      temp[0] = current_user.euid == 0 ? '#' : '$';
	      temp[1] = '\0';
	      goto add_string;

#if defined (READLINE)
	    case '[':
	    case ']':
	      temp = xmalloc (3);
	      temp[0] = '\001';
	      temp[1] = (c == '[') ? RL_PROMPT_START_IGNORE : RL_PROMPT_END_IGNORE;
	      temp[2] = '\0';
	      goto add_string;
#endif /* READLINE */

	    case '\\':
	      temp = xmalloc (2);
	      temp[0] = c;
	      temp[1] = '\0';
	      goto add_string;

	    case 'a':
	    case 'e':
	      temp = xmalloc (2);
	      temp[0] = (c == 'a') ? '\07' : '\033';
	      temp[1] = '\0';
	      goto add_string;

	    default:
	      temp = xmalloc (3);
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
      list = expand_string_unsplit (result, Q_DOUBLE_QUOTES);
      free (result);
      result = string_list (list);
      dispose_words (list);
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

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
int
yyerror ()
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
  return (0);
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
static void
report_syntax_error (message)
     char *message;
{
  char *msg, *t;
  int token_end, i;
  char msg2[2];

  if (message)
    {
      parser_error (line_number, "%s", message);
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
      last_command_exit_value = EX_USAGE;
      return;
    }

  /* If the line of input we're reading is not null, try to find the
     objectionable token. */
  if (shell_input_line && *shell_input_line)
    {
      t = shell_input_line;
      i = shell_input_line_index;
      token_end = 0;

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

      /* Print the offending token. */
      if (token_end || (i == 0 && token_end == 0))
	{
	  if (token_end)
	    {
	      msg = xmalloc (1 + (token_end - i));
	      strncpy (msg, t + i, token_end - i);
	      msg[token_end - i] = '\0';
	    }
	  else	/* one-character token */
	    {
	      msg2[0] = t[i];
	      msg2[1] = '\0';
	      msg = msg2;
	    }

	  parser_error (line_number, "syntax error near unexpected token `%s'", msg);

	  if (msg != msg2)
	    free (msg);
	}

      /* If not interactive, print the line containing the error. */
      if (interactive == 0)
	{
	  msg = savestring (shell_input_line);
	  token_end = strlen (msg);
	  while (token_end && msg[token_end - 1] == '\n')
	    msg[--token_end] = '\0';

	  parser_error (line_number, "`%s'", msg);
	  free (msg);
	}
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
   (dispose_command () will actually free the command. */
static void
discard_parser_constructs (error_p)
     int error_p;
{
}

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
	      /* Reset the prompt string to be $PS1. */
	      prompt_string_pointer = (char **)NULL;
	      prompt_again ();
	      last_read_token = current_token = '\n';
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
