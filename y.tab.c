
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
#define	WORD	274
#define	ASSIGNMENT_WORD	275
#define	NUMBER	276
#define	AND_AND	277
#define	OR_OR	278
#define	GREATER_GREATER	279
#define	LESS_LESS	280
#define	LESS_AND	281
#define	GREATER_AND	282
#define	SEMI_SEMI	283
#define	LESS_LESS_MINUS	284
#define	AND_GREATER	285
#define	LESS_GREATER	286
#define	GREATER_BAR	287
#define	yacc_EOF	288

#line 21 "./parse.y"

#include <stdio.h>
#include "bashtypes.h"
#include <signal.h>
#include "bashansi.h"
#include "shell.h"
#include "flags.h"
#include "input.h"

#if defined (READLINE)
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

#define YYDEBUG 1
extern int eof_encountered;
extern int no_line_editing;
extern int current_command_number;
extern int interactive, interactive_shell, login_shell;
extern int posixly_correct;
extern int last_command_exit_value;
extern int interrupt_immediately;
extern char *shell_name, *current_host_name;
extern Function *last_shell_builtin, *this_shell_builtin;
#if defined (READLINE)
extern int bash_readline_initialized;
#endif
#if defined (BUFFERED_INPUT)
extern int bash_input_fd_changed;
#endif

/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

/* This is kind of sickening.  In order to let these variables be seen by
   all the functions that need them, I am forced to place their declarations
   far away from the place where they should logically be found. */

static int reserved_word_acceptable ();
static int read_token ();

static void report_syntax_error ();
static void handle_eof_input_unit ();
static void prompt_again ();
static void reset_readline_prompt ();
static void print_prompt ();

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* The decoded prompt string.  Used if READLINE is not defined or if
   editing is turned off.  Analogous to current_readline_prompt. */
static char *current_decoded_prompt;

/* The number of lines read from input while creating the current command. */
int current_command_line_count = 0;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
static REDIRECT *redir_stack[10];
int need_here_doc = 0;

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
static char *shell_input_line = (char *)NULL;
static int shell_input_line_index = 0;
static int shell_input_line_size = 0;	/* Amount allocated for shell_input_line. */
static int shell_input_line_len = 0;	/* strlen (shell_input_line) */

/* Either zero or EOF. */
static int shell_input_line_terminator = 0;

static REDIRECTEE redir;

#line 122 "./parse.y"
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



#define	YYFINAL		258
#define	YYFLAG		-32768
#define	YYNTBASE	45

#define YYTRANSLATE(x) ((unsigned)(x) <= 288 ? yytranslate[x] : 73)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    35,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    33,     2,    43,
    44,     2,     2,     2,    40,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    34,    39,
     2,    38,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    41,    37,    42,     2,     2,     2,     2,     2,
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
    26,    27,    28,    29,    30,    31,    32,    36
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     5,     8,    10,    11,    14,    17,    20,    24,
    28,    31,    35,    38,    42,    45,    49,    52,    56,    59,
    63,    66,    70,    73,    77,    80,    84,    87,    91,    94,
    98,   101,   104,   108,   110,   112,   114,   116,   119,   121,
   124,   126,   128,   130,   133,   140,   147,   155,   163,   174,
   185,   192,   200,   207,   213,   219,   221,   223,   225,   227,
   229,   236,   243,   251,   259,   270,   281,   287,   294,   301,
   309,   314,   320,   324,   330,   338,   345,   349,   354,   361,
   367,   369,   372,   377,   382,   388,   394,   396,   399,   405,
   411,   418,   425,   427,   431,   434,   436,   440,   444,   448,
   453,   458,   463,   468,   473,   475,   478,   480,   482,   484,
   485,   488,   490,   493,   496,   501,   506,   510,   514,   516,
   519,   524
};

static const short yyrhs[] = {    70,
    35,     0,    35,     0,     1,    35,     0,    36,     0,     0,
    46,    19,     0,    38,    19,     0,    39,    19,     0,    21,
    38,    19,     0,    21,    39,    19,     0,    24,    19,     0,
    21,    24,    19,     0,    25,    19,     0,    21,    25,    19,
     0,    26,    21,     0,    21,    26,    21,     0,    27,    21,
     0,    21,    27,    21,     0,    26,    19,     0,    21,    26,
    19,     0,    27,    19,     0,    21,    27,    19,     0,    29,
    19,     0,    21,    29,    19,     0,    27,    40,     0,    21,
    27,    40,     0,    26,    40,     0,    21,    26,    40,     0,
    30,    19,     0,    21,    31,    19,     0,    31,    19,     0,
    32,    19,     0,    21,    32,    19,     0,    19,     0,    20,
     0,    47,     0,    47,     0,    49,    47,     0,    48,     0,
    50,    48,     0,    50,     0,    52,     0,    53,     0,    53,
    49,     0,    10,    19,    69,    14,    65,    15,     0,    10,
    19,    69,    41,    65,    42,     0,    10,    19,    34,    69,
    14,    65,    15,     0,    10,    19,    34,    69,    41,    65,
    42,     0,    10,    19,    69,    17,    46,    68,    69,    14,
    65,    15,     0,    10,    19,    69,    17,    46,    68,    69,
    41,    65,    42,     0,     8,    19,    69,    17,    69,     9,
     0,     8,    19,    69,    17,    62,    69,     9,     0,     8,
    19,    69,    17,    60,     9,     0,    12,    65,    14,    65,
    15,     0,    13,    65,    14,    65,    15,     0,    54,     0,
    57,     0,    56,     0,    58,     0,    55,     0,    11,    19,
    69,    14,    65,    15,     0,    11,    19,    69,    41,    65,
    42,     0,    11,    19,    34,    69,    14,    65,    15,     0,
    11,    19,    34,    69,    41,    65,    42,     0,    11,    19,
    69,    17,    46,    68,    69,    14,    65,    15,     0,    11,
    19,    69,    17,    46,    68,    69,    41,    65,    42,     0,
    19,    43,    44,    69,    58,     0,    19,    43,    44,    69,
    58,    49,     0,    16,    19,    43,    44,    69,    58,     0,
    16,    19,    43,    44,    69,    58,    49,     0,    16,    19,
    69,    58,     0,    16,    19,    69,    58,    49,     0,    43,
    65,    44,     0,     3,    65,     4,    65,     7,     0,     3,
    65,     4,    65,     5,    65,     7,     0,     3,    65,     4,
    65,    59,     7,     0,    41,    65,    42,     0,     6,    65,
     4,    65,     0,     6,    65,     4,    65,     5,    65,     0,
     6,    65,     4,    65,    59,     0,    61,     0,    62,    61,
     0,    69,    64,    44,    65,     0,    69,    64,    44,    69,
     0,    69,    43,    64,    44,    65,     0,    69,    43,    64,
    44,    69,     0,    63,     0,    62,    63,     0,    69,    64,
    44,    65,    28,     0,    69,    64,    44,    69,    28,     0,
    69,    43,    64,    44,    65,    28,     0,    69,    43,    64,
    44,    69,    28,     0,    19,     0,    64,    37,    19,     0,
    69,    66,     0,    67,     0,    67,    35,    69,     0,    67,
    33,    69,     0,    67,    34,    69,     0,    67,    22,    69,
    67,     0,    67,    23,    69,    67,     0,    67,    33,    69,
    67,     0,    67,    34,    69,    67,     0,    67,    35,    69,
    67,     0,    72,     0,    18,    72,     0,    35,     0,    34,
     0,    36,     0,     0,    69,    35,     0,    71,     0,    71,
    33,     0,    71,    34,     0,    71,    22,    69,    71,     0,
    71,    23,    69,    71,     0,    71,    33,    71,     0,    71,
    34,    71,     0,    72,     0,    18,    72,     0,    72,    37,
    69,    72,     0,    51,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   163,   172,   179,   195,   205,   207,   211,   216,   221,   226,
   231,   236,   241,   247,   253,   258,   263,   268,   273,   278,
   283,   288,   293,   300,   307,   312,   317,   322,   327,   332,
   337,   353,   358,   365,   367,   369,   373,   377,   388,   390,
   394,   396,   400,   402,   417,   419,   421,   423,   425,   427,
   430,   432,   434,   436,   438,   440,   442,   444,   446,   448,
   452,   458,   464,   470,   476,   482,   490,   493,   496,   499,
   502,   505,   509,   513,   515,   517,   522,   526,   528,   530,
   534,   535,   539,   541,   543,   545,   549,   550,   554,   556,
   558,   560,   564,   566,   575,   583,   584,   585,   592,   596,
   598,   600,   607,   609,   611,   613,   620,   621,   622,   625,
   626,   635,   641,   650,   658,   660,   662,   669,   671,   673,
   680,   683
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","IF","THEN",
"ELSE","ELIF","FI","CASE","ESAC","FOR","SELECT","WHILE","UNTIL","DO","DONE",
"FUNCTION","IN","BANG","WORD","ASSIGNMENT_WORD","NUMBER","AND_AND","OR_OR","GREATER_GREATER",
"LESS_LESS","LESS_AND","GREATER_AND","SEMI_SEMI","LESS_LESS_MINUS","AND_GREATER",
"LESS_GREATER","GREATER_BAR","'&'","';'","'\\n'","yacc_EOF","'|'","'>'","'<'",
"'-'","'{'","'}'","'('","')'","inputunit","words","redirection","simple_command_element",
"redirections","simple_command","command","shell_command","shell_command_1",
"select_command","function_def","subshell","if_command","group_command","elif_clause",
"case_clause_1","pattern_list_1","case_clause_sequence","pattern_list","pattern",
"list","list0","list1","list_terminator","newlines","simple_list","simple_list1",
"pipeline", NULL
};
#endif

static const short yyr1[] = {     0,
    45,    45,    45,    45,    46,    46,    47,    47,    47,    47,
    47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
    47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
    47,    47,    47,    48,    48,    48,    49,    49,    50,    50,
    51,    51,    52,    52,    53,    53,    53,    53,    53,    53,
    53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
    54,    54,    54,    54,    54,    54,    55,    55,    55,    55,
    55,    55,    56,    57,    57,    57,    58,    59,    59,    59,
    60,    60,    61,    61,    61,    61,    62,    62,    63,    63,
    63,    63,    64,    64,    65,    66,    66,    66,    66,    67,
    67,    67,    67,    67,    67,    67,    68,    68,    68,    69,
    69,    70,    70,    70,    71,    71,    71,    71,    71,    71,
    72,    72
};

static const short yyr2[] = {     0,
     2,     1,     2,     1,     0,     2,     2,     2,     3,     3,
     2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
     2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
     2,     2,     3,     1,     1,     1,     1,     2,     1,     2,
     1,     1,     1,     2,     6,     6,     7,     7,    10,    10,
     6,     7,     6,     5,     5,     1,     1,     1,     1,     1,
     6,     6,     7,     7,    10,    10,     5,     6,     6,     7,
     4,     5,     3,     5,     7,     6,     3,     4,     6,     5,
     1,     2,     4,     4,     5,     5,     1,     2,     5,     5,
     6,     6,     1,     3,     2,     1,     3,     3,     3,     4,
     4,     4,     4,     4,     1,     2,     1,     1,     1,     0,
     2,     1,     2,     2,     4,     4,     3,     3,     1,     2,
     4,     1
};

static const short yydefact[] = {     0,
     0,   110,     0,     0,     0,   110,   110,     0,     0,    34,
    35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     2,     4,     0,     0,   110,   110,    36,    39,    41,   122,
    42,    43,    56,    60,    58,    57,    59,     0,   112,   119,
     3,     0,     0,   110,   110,   110,     0,     0,   110,   120,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    11,    13,    19,    15,    27,    21,    17,    25,    23,    29,
    31,    32,     7,     8,     0,     0,    34,    40,    37,    44,
     1,   110,   110,   113,   114,   110,   110,     0,   111,    95,
    96,   105,     0,   110,     0,   110,     0,   110,   110,     0,
     0,   110,    12,    14,    20,    16,    28,    22,    18,    26,
    24,    30,    33,     9,    10,    77,    73,    38,     0,     0,
   117,   118,     0,     0,   106,   110,   110,   110,   110,   110,
   110,     0,   110,     5,   110,     0,   110,     5,   110,     0,
     0,   110,    71,     0,   115,   116,     0,     0,   121,   110,
   110,    74,     0,     0,     0,    98,    99,    97,     0,    81,
   110,    87,     0,   110,   110,     0,     0,     0,   110,   110,
     0,     0,     0,    54,    55,     0,    72,    67,     0,     0,
    76,   100,   101,   102,   103,   104,    53,    82,    88,     0,
    51,    93,     0,     0,     0,     0,    45,     6,   108,   107,
   109,   110,    46,     0,     0,    61,   110,    62,    69,    68,
    75,   110,   110,   110,   110,    52,     0,     0,   110,    47,
    48,     0,    63,    64,     0,    70,    78,     0,     0,     0,
   110,    94,    83,    84,   110,   110,   110,   110,   110,    80,
    85,    86,    89,    90,     0,     0,     0,     0,    79,    91,
    92,    49,    50,    65,    66,     0,     0,     0
};

static const short yydefgoto[] = {   256,
   167,    27,    28,    80,    29,    30,    31,    32,    33,    34,
    35,    36,    37,   153,   159,   160,   161,   162,   194,    42,
    90,    91,   202,    43,    38,   121,    92
};

static const short yypact[] = {   233,
   -28,-32768,     2,    10,    15,-32768,-32768,    32,   437,    19,
-32768,   494,    46,    52,    -5,    39,    59,    61,    93,    95,
-32768,-32768,   102,   103,-32768,-32768,-32768,-32768,   462,-32768,
-32768,   478,-32768,-32768,-32768,-32768,-32768,    71,   116,    91,
-32768,   126,   301,-32768,   117,   118,   123,   139,    89,    91,
   111,   137,   138,    75,    76,   141,   143,   146,   148,   149,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   127,   131,-32768,-32768,-32768,   478,
-32768,-32768,-32768,   369,   369,-32768,-32768,   437,-32768,-32768,
   101,    91,    37,-32768,    -4,-32768,    22,-32768,-32768,   133,
   -23,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   335,   335,
    60,    60,   403,    98,    91,-32768,-32768,-32768,-32768,-32768,
-32768,     3,-32768,-32768,-32768,    33,-32768,-32768,-32768,   167,
   172,-32768,   478,   -23,-32768,-32768,   369,   369,    91,-32768,
-32768,-32768,   181,   301,   301,   301,   301,   301,   186,-32768,
-32768,-32768,    21,-32768,-32768,   192,    83,   168,-32768,-32768,
   194,    83,   175,-32768,-32768,   -23,   478,   478,   208,   214,
-32768,-32768,-32768,    87,    87,    87,-32768,-32768,-32768,    24,
-32768,-32768,   200,   -22,   205,   179,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   207,   182,-32768,-32768,-32768,   478,   478,
-32768,-32768,-32768,-32768,-32768,-32768,    29,   204,-32768,-32768,
-32768,    34,-32768,-32768,    35,   478,   135,   301,   301,   301,
-32768,-32768,   198,   173,-32768,-32768,-32768,-32768,-32768,-32768,
   199,   267,-32768,-32768,   213,   193,   222,   196,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   239,   240,-32768
};

static const short yypgoto[] = {-32768,
   104,   -30,   218,  -132,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   -92,    28,-32768,   100,-32768,   105,    55,    -6,
-32768,  -130,    78,   -41,-32768,     6,    23
};


#define	YYLAST		533


static const short yytable[] = {    47,
    48,    79,    93,    95,    97,    39,    41,   101,   143,   133,
   177,    89,   134,    63,   218,    64,   164,    25,    75,    76,
    44,   219,    40,   182,   183,   184,   185,   186,    45,   191,
    89,    50,   216,    46,    65,   137,   135,    89,   138,   192,
   119,   120,   192,   165,   123,   210,   169,   235,   237,   118,
    49,   178,   132,   131,   136,    89,    89,    66,    89,    67,
   144,    51,   139,   193,    61,   218,   193,    89,    89,    89,
    62,    89,   231,   170,   236,   238,   226,    69,    68,    70,
   124,    82,    83,   209,   154,   155,   156,   157,   158,   163,
   122,   140,   141,   105,   108,   106,   109,   184,   185,   186,
   176,   198,   150,   151,   152,    81,    40,    40,   126,   127,
   125,    71,    79,    72,   107,   110,   199,   200,   201,   190,
    73,    74,   126,   127,   145,   146,   166,    86,   168,    87,
   171,   100,   173,   128,   129,   130,    98,    82,    83,   239,
   151,    40,    40,   179,   180,   149,   118,    79,    84,    85,
    94,    96,    99,   122,   102,   103,   104,   195,   196,   111,
   222,   112,   204,   205,   113,   225,   114,   115,   116,    40,
    40,   228,   229,   230,   117,     2,   142,   234,    79,   118,
     3,   174,     4,     5,     6,     7,   175,   181,     8,   242,
    88,    10,    11,    12,   187,   118,    13,    14,    15,    16,
   244,    17,    18,    19,    20,   227,   197,    89,   206,   203,
    23,    24,   233,    25,   211,    26,   208,   212,   192,   220,
   221,   223,   232,   224,   241,   243,   250,   252,   245,   246,
   247,   248,   249,     1,   253,     2,   254,   255,   257,   258,
     3,   172,     4,     5,     6,     7,    78,   217,     8,   207,
     9,    10,    11,    12,   240,     0,    13,    14,    15,    16,
   188,    17,    18,    19,    20,   189,     0,    21,    22,     2,
    23,    24,     0,    25,     3,    26,     4,     5,     6,     7,
     0,     0,     8,     0,    88,    10,    11,    12,     0,     0,
    13,    14,    15,    16,   251,    17,    18,    19,    20,     0,
     0,    89,     0,     2,    23,    24,     0,    25,     3,    26,
     4,     5,     6,     7,     0,     0,     8,     0,    88,    10,
    11,    12,     0,     0,    13,    14,    15,    16,     0,    17,
    18,    19,    20,     0,     0,    89,     0,     2,    23,    24,
     0,    25,     3,    26,     4,     5,     6,     7,     0,     0,
     8,     0,     9,    10,    11,    12,     0,     0,    13,    14,
    15,    16,     0,    17,    18,    19,    20,     0,     0,    89,
     0,     2,    23,    24,     0,    25,     3,    26,     4,     5,
     6,     7,     0,     0,     8,     0,     9,    10,    11,    12,
     0,     0,    13,    14,    15,    16,     0,    17,    18,    19,
    20,     0,     0,     0,     0,     2,    23,    24,     0,    25,
     3,    26,     4,     5,     6,     7,     0,     0,     8,     0,
     0,    10,    11,    12,     0,     0,    13,    14,    15,    16,
     0,    17,    18,    19,    20,     0,     0,    89,     0,     2,
    23,    24,     0,    25,     3,    26,     4,     5,     6,     7,
     0,     0,     8,     0,     0,    10,    11,    12,     0,     0,
    13,    14,    15,    16,     0,    17,    18,    19,    20,     0,
     0,     0,     0,     0,    23,    24,     0,    25,     0,    26,
    77,    11,    12,     0,     0,    13,    14,    15,    16,     0,
    17,    18,    19,    20,     0,     0,     0,     0,    12,    23,
    24,    13,    14,    15,    16,     0,    17,    18,    19,    20,
     0,     0,     0,     0,     0,    23,    24,    52,    53,    54,
    55,     0,    56,     0,    57,    58,     0,     0,     0,     0,
     0,    59,    60
};

static const short yycheck[] = {     6,
     7,    32,    44,    45,    46,     0,    35,    49,   101,    14,
   143,    35,    17,    19,    37,    21,    14,    41,    25,    26,
    19,    44,     0,   154,   155,   156,   157,   158,    19,     9,
    35,     9,     9,    19,    40,    14,    41,    35,    17,    19,
    82,    83,    19,    41,    86,   178,    14,    14,    14,    80,
    19,   144,    94,    17,    96,    35,    35,    19,    35,    21,
   102,    43,    41,    43,    19,    37,    43,    35,    35,    35,
    19,    35,    44,    41,    41,    41,   209,    19,    40,    19,
    87,    22,    23,   176,   126,   127,   128,   129,   130,   131,
    85,    98,    99,    19,    19,    21,    21,   228,   229,   230,
   142,    19,     5,     6,     7,    35,    84,    85,    22,    23,
    88,    19,   143,    19,    40,    40,    34,    35,    36,   161,
    19,    19,    22,    23,   119,   120,   133,    37,   135,     4,
   137,    43,   139,    33,    34,    35,    14,    22,    23,     5,
     6,   119,   120,   150,   151,   123,   177,   178,    33,    34,
    34,    34,    14,   148,    44,    19,    19,   164,   165,    19,
   202,    19,   169,   170,    19,   207,    19,    19,    42,   147,
   148,   213,   214,   215,    44,     3,    44,   219,   209,   210,
     8,    15,    10,    11,    12,    13,    15,     7,    16,   231,
    18,    19,    20,    21,     9,   226,    24,    25,    26,    27,
    28,    29,    30,    31,    32,   212,    15,    35,    15,    42,
    38,    39,   219,    41,     7,    43,    42,     4,    19,    15,
    42,    15,    19,    42,   231,    28,    28,    15,   235,   236,
   237,   238,   239,     1,    42,     3,    15,    42,     0,     0,
     8,   138,    10,    11,    12,    13,    29,   193,    16,   172,
    18,    19,    20,    21,   227,    -1,    24,    25,    26,    27,
   161,    29,    30,    31,    32,   161,    -1,    35,    36,     3,
    38,    39,    -1,    41,     8,    43,    10,    11,    12,    13,
    -1,    -1,    16,    -1,    18,    19,    20,    21,    -1,    -1,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    -1,
    -1,    35,    -1,     3,    38,    39,    -1,    41,     8,    43,
    10,    11,    12,    13,    -1,    -1,    16,    -1,    18,    19,
    20,    21,    -1,    -1,    24,    25,    26,    27,    -1,    29,
    30,    31,    32,    -1,    -1,    35,    -1,     3,    38,    39,
    -1,    41,     8,    43,    10,    11,    12,    13,    -1,    -1,
    16,    -1,    18,    19,    20,    21,    -1,    -1,    24,    25,
    26,    27,    -1,    29,    30,    31,    32,    -1,    -1,    35,
    -1,     3,    38,    39,    -1,    41,     8,    43,    10,    11,
    12,    13,    -1,    -1,    16,    -1,    18,    19,    20,    21,
    -1,    -1,    24,    25,    26,    27,    -1,    29,    30,    31,
    32,    -1,    -1,    -1,    -1,     3,    38,    39,    -1,    41,
     8,    43,    10,    11,    12,    13,    -1,    -1,    16,    -1,
    -1,    19,    20,    21,    -1,    -1,    24,    25,    26,    27,
    -1,    29,    30,    31,    32,    -1,    -1,    35,    -1,     3,
    38,    39,    -1,    41,     8,    43,    10,    11,    12,    13,
    -1,    -1,    16,    -1,    -1,    19,    20,    21,    -1,    -1,
    24,    25,    26,    27,    -1,    29,    30,    31,    32,    -1,
    -1,    -1,    -1,    -1,    38,    39,    -1,    41,    -1,    43,
    19,    20,    21,    -1,    -1,    24,    25,    26,    27,    -1,
    29,    30,    31,    32,    -1,    -1,    -1,    -1,    21,    38,
    39,    24,    25,    26,    27,    -1,    29,    30,    31,    32,
    -1,    -1,    -1,    -1,    -1,    38,    39,    24,    25,    26,
    27,    -1,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,
    -1,    38,    39
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
#line 164 "./parse.y"
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
#line 173 "./parse.y"
{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			;
    break;}
case 3:
#line 181 "./parse.y"
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
#line 196 "./parse.y"
{
			  /* Case of EOF seen by itself.  Do ignoreeof or 
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			;
    break;}
case 5:
#line 206 "./parse.y"
{ yyval.word_list = (WORD_LIST *)NULL; ;
    break;}
case 6:
#line 208 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-1].word_list); ;
    break;}
case 7:
#line 212 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_direction, redir);
			;
    break;}
case 8:
#line 217 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_direction, redir);
			;
    break;}
case 9:
#line 222 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_direction, redir);
			;
    break;}
case 10:
#line 227 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_direction, redir);
			;
    break;}
case 11:
#line 232 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_appending_to, redir);
			;
    break;}
case 12:
#line 237 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_appending_to, redir);
			;
    break;}
case 13:
#line 242 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 14:
#line 248 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 15:
#line 254 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (0, r_duplicating_input, redir);
			;
    break;}
case 16:
#line 259 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input, redir);
			;
    break;}
case 17:
#line 264 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (1, r_duplicating_output, redir);
			;
    break;}
case 18:
#line 269 "./parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output, redir);
			;
    break;}
case 19:
#line 274 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_duplicating_input_word, redir);
			;
    break;}
case 20:
#line 279 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input_word, redir);
			;
    break;}
case 21:
#line 284 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_duplicating_output_word, redir);
			;
    break;}
case 22:
#line 289 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output_word, redir);
			;
    break;}
case 23:
#line 294 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (0, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 24:
#line 301 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			;
    break;}
case 25:
#line 308 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (1, r_close_this, redir);
			;
    break;}
case 26:
#line 313 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			;
    break;}
case 27:
#line 318 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (0, r_close_this, redir);
			;
    break;}
case 28:
#line 323 "./parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			;
    break;}
case 29:
#line 328 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_err_and_out, redir);
			;
    break;}
case 30:
#line 333 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_output, redir);
			;
    break;}
case 31:
#line 338 "./parse.y"
{
			  REDIRECT *t1, *t2;

			  redir.filename = yyvsp[0].word;
			  if (posixly_correct)
			    yyval.redirect = make_redirection (0, r_input_output, redir);
			  else
			    {
			      t1 = make_redirection (0, r_input_direction, redir);
			      redir.filename = copy_word (yyvsp[0].word);
			      t2 = make_redirection (1, r_output_direction, redir);
			      t1->next = t2;
			      yyval.redirect = t1;
			    }
			;
    break;}
case 32:
#line 354 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_force, redir);
			;
    break;}
case 33:
#line 359 "./parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_force, redir);
			;
    break;}
case 34:
#line 366 "./parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; ;
    break;}
case 35:
#line 368 "./parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; ;
    break;}
case 36:
#line 370 "./parse.y"
{ yyval.element.redirect = yyvsp[0].redirect; yyval.element.word = 0; ;
    break;}
case 37:
#line 374 "./parse.y"
{
			  yyval.redirect = yyvsp[0].redirect;
			;
    break;}
case 38:
#line 378 "./parse.y"
{ 
			  register REDIRECT *t = yyvsp[-1].redirect;

			  while (t->next)
			    t = t->next;
			  t->next = yyvsp[0].redirect; 
			  yyval.redirect = yyvsp[-1].redirect;
			;
    break;}
case 39:
#line 389 "./parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, (COMMAND *)NULL); ;
    break;}
case 40:
#line 391 "./parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, yyvsp[-1].command); ;
    break;}
case 41:
#line 395 "./parse.y"
{ yyval.command = clean_simple_command (yyvsp[0].command); ;
    break;}
case 42:
#line 397 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 43:
#line 401 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 44:
#line 403 "./parse.y"
{
			  if (yyvsp[-1].command->redirects)
			    {
			      register REDIRECT *t;
			      for (t = yyvsp[-1].command->redirects; t->next; t = t->next)
				;
			      t->next = yyvsp[0].redirect;
			    }
			  else
			    yyvsp[-1].command->redirects = yyvsp[0].redirect;
			  yyval.command = yyvsp[-1].command;
			;
    break;}
case 45:
#line 418 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 46:
#line 420 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 47:
#line 422 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 48:
#line 424 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); ;
    break;}
case 49:
#line 426 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); ;
    break;}
case 50:
#line 428 "./parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); ;
    break;}
case 51:
#line 431 "./parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, (PATTERN_LIST *)NULL); ;
    break;}
case 52:
#line 433 "./parse.y"
{ yyval.command = make_case_command (yyvsp[-5].word, yyvsp[-2].pattern); ;
    break;}
case 53:
#line 435 "./parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, yyvsp[-1].pattern); ;
    break;}
case 54:
#line 437 "./parse.y"
{ yyval.command = make_while_command (yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 55:
#line 439 "./parse.y"
{ yyval.command = make_until_command (yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 56:
#line 441 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 57:
#line 443 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 58:
#line 445 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 59:
#line 447 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 60:
#line 449 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 61:
#line 453 "./parse.y"
{
#if defined (SELECT_COMMAND)
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
#endif
			;
    break;}
case 62:
#line 459 "./parse.y"
{
#if defined (SELECT_COMMAND)
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command);
#endif
			;
    break;}
case 63:
#line 465 "./parse.y"
{
#if defined (SELECT_COMMAND)
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
#endif
			;
    break;}
case 64:
#line 471 "./parse.y"
{
#if defined (SELECT_COMMAND)
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
#endif
			;
    break;}
case 65:
#line 477 "./parse.y"
{
#if defined (SELECT_COMMAND)
			  yyval.command = make_select_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command);
#endif
			;
    break;}
case 66:
#line 483 "./parse.y"
{
#if defined (SELECT_COMMAND)
			  yyval.command = make_select_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command);
#endif
			;
    break;}
case 67:
#line 491 "./parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command); ;
    break;}
case 68:
#line 494 "./parse.y"
{ yyvsp[-1].command->redirects = yyvsp[0].redirect; yyval.command = make_function_def (yyvsp[-5].word, yyvsp[-1].command); ;
    break;}
case 69:
#line 497 "./parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command); ;
    break;}
case 70:
#line 500 "./parse.y"
{ yyvsp[-1].command->redirects = yyvsp[0].redirect; yyval.command = make_function_def (yyvsp[-5].word, yyvsp[-1].command); ;
    break;}
case 71:
#line 503 "./parse.y"
{ yyval.command = make_function_def (yyvsp[-2].word, yyvsp[0].command); ;
    break;}
case 72:
#line 506 "./parse.y"
{ yyvsp[-1].command->redirects = yyvsp[0].redirect; yyval.command = make_function_def (yyvsp[-3].word, yyvsp[-1].command); ;
    break;}
case 73:
#line 510 "./parse.y"
{ yyvsp[-1].command->flags |= CMD_WANT_SUBSHELL; yyval.command = yyvsp[-1].command; ;
    break;}
case 74:
#line 514 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, (COMMAND *)NULL); ;
    break;}
case 75:
#line 516 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-5].command, yyvsp[-3].command, yyvsp[-1].command); ;
    break;}
case 76:
#line 518 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[-1].command); ;
    break;}
case 77:
#line 523 "./parse.y"
{ yyval.command = make_group_command (yyvsp[-1].command); ;
    break;}
case 78:
#line 527 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-2].command, yyvsp[0].command, (COMMAND *)NULL); ;
    break;}
case 79:
#line 529 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[0].command); ;
    break;}
case 80:
#line 531 "./parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, yyvsp[0].command); ;
    break;}
case 82:
#line 536 "./parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; ;
    break;}
case 83:
#line 540 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); ;
    break;}
case 84:
#line 542 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); ;
    break;}
case 85:
#line 544 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); ;
    break;}
case 86:
#line 546 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); ;
    break;}
case 88:
#line 551 "./parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; ;
    break;}
case 89:
#line 555 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, yyvsp[-1].command); ;
    break;}
case 90:
#line 557 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, (COMMAND *)NULL); ;
    break;}
case 91:
#line 559 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, yyvsp[-1].command); ;
    break;}
case 92:
#line 561 "./parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-3].word_list, (COMMAND *)NULL); ;
    break;}
case 93:
#line 565 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); ;
    break;}
case 94:
#line 567 "./parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-2].word_list); ;
    break;}
case 95:
#line 576 "./parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			 ;
    break;}
case 98:
#line 586 "./parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, (COMMAND *)NULL, '&');
			;
    break;}
case 100:
#line 597 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); ;
    break;}
case 101:
#line 599 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); ;
    break;}
case 102:
#line 601 "./parse.y"
{
			  if (yyvsp[-3].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-3].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '&');
			;
    break;}
case 103:
#line 608 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); ;
    break;}
case 104:
#line 610 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); ;
    break;}
case 105:
#line 612 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 106:
#line 614 "./parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 112:
#line 636 "./parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			;
    break;}
case 113:
#line 642 "./parse.y"
{
			  if (yyvsp[-1].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents ();
			;
    break;}
case 114:
#line 651 "./parse.y"
{
			  yyval.command = yyvsp[-1].command;
			  if (need_here_doc)
			    gather_here_documents ();
			;
    break;}
case 115:
#line 659 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); ;
    break;}
case 116:
#line 661 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); ;
    break;}
case 117:
#line 663 "./parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, '&');
			;
    break;}
case 118:
#line 670 "./parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, ';'); ;
    break;}
case 119:
#line 672 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
    break;}
case 120:
#line 674 "./parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			;
    break;}
case 121:
#line 682 "./parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '|'); ;
    break;}
case 122:
#line 684 "./parse.y"
{ yyval.command = yyvsp[0].command; ;
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
#line 686 "./parse.y"


/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_GROW_SIZE 512

/* The token currently being read. */
static int current_token = 0;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
static int last_read_token = 0;

/* The token read prior to last_read_token. */
static int token_before_that = 0;

/* If non-zero, it is the token that we want read_token to return
   regardless of what text is (or isn't) present to be read.  This
   is reset by read_token. */
static int token_to_read = 0;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget, type, location) makes the function GET the
   installed function for getting the next character, makes UNGET the
   installed function for un-getting a character, sets the type of stream
   (either string or file) from TYPE, and makes LOCATION point to where
   the input is coming from. */

/* Unconditionally returns end-of-file. */
return_EOF ()
{
  return (EOF);
}

/* Variable containing the current get and unget functions.
   See ./input.h for a clearer description. */
BASH_INPUT bash_input;

/* Set all of the fields in BASH_INPUT to NULL. */
void
initialize_bash_input ()
{
  bash_input.type = 0;
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
     int type;
     char *name;
     INPUT_STREAM location;
{
  bash_input.type = type;
  FREE (bash_input.name);

  if (name)
    bash_input.name = savestring (name);
  else
    bash_input.name = (char *)NULL;

#if defined (CRAY)
  memcpy((char *)&bash_input.location.string, (char *)&location.string, sizeof(location));
#else
  bash_input.location = location;
#endif
  bash_input.getter = get;
  bash_input.ungetter = unget;
}

/* Call this to get the next character of input. */
yy_getc ()
{
  return (*(bash_input.getter)) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
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
  if (!current_readline_line)
    {
      SigHandler *old_sigint;
      int line_len;

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

      if (!current_readline_prompt)
	current_readline_line = readline ("");
      else
	current_readline_line = readline (current_readline_prompt);

      if (signal_is_ignored (SIGINT) == 0)
	{
	  interrupt_immediately--;
	  set_signal_handler (SIGINT, old_sigint);
	}

      /* Reset the prompt to whatever is in the decoded value of
	 prompt_string_pointer. */
      reset_readline_prompt ();

      current_readline_line_index = 0;

      if (!current_readline_line)
	return (EOF);

      line_len = strlen (current_readline_line);
      current_readline_line = xrealloc (current_readline_line, 2 + line_len);
      current_readline_line[line_len++] = '\n';
      current_readline_line[line_len] = '\0';
    }

  if (!current_readline_line[current_readline_line_index])
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      int c = (unsigned char)current_readline_line[current_readline_line_index++];
      return (c);
    }
}

static int
yy_readline_unget (c)
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
  register unsigned char *string;
  register int c;

  string = bash_input.location.string;
  c = EOF;

  /* If the string doesn't exist, or is empty, EOF found. */
  if (string && *string)
    {
      c = *string++;
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
     char *string;
     char *name;
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
#if defined (NO_READ_RESTART_ON_SIGNAL)
    result = (unsigned char)getc_with_restart (bash_input.location.file);
#else
    result = (unsigned char)getc (bash_input.location.file);
#endif /* !NO_READ_RESTART_ON_SIGNAL */
  return (result);
}

static int
yy_stream_unget (c)
     int c;
{
#if defined (NO_READ_RESTART_ON_SIGNAL)
  return (ungetc_with_restart (c, bash_input.location.file));
#else
  return (ungetc (c, bash_input.location.file));
#endif
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

push_stream ()
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
  EOF_Reached = line_number = 0;
}

pop_stream ()
{
  int temp;

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
     int type;
{
  register STREAM_SAVER *s;
 
  for (s = stream_list; s; s = s->next)
    if (s->bash_input.type == type)
      return 1;
  return 0;
}


/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list'
 * is:
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */
static int in_case_pattern_list = 0;

#if defined (ALIAS)
/*
 * Pseudo-global variables used in implementing token-wise alias expansion.
 */

static int expand_next_token = 0;

/*
 * Pushing and popping strings.  This works together with shell_getc to 
 * implement alias expansion on a per-token basis.
 */

typedef struct string_saver {
  struct string_saver *next;
  int expand_alias;  /* Value to set expand_alias to when string is popped. */
  char *saved_line;
  int saved_line_size, saved_line_index, saved_line_terminator;
} STRING_SAVER;

STRING_SAVER *pushed_string_list = (STRING_SAVER *)NULL;

static void save_expansion ();

/*
 * Push the current shell_input_line onto a stack of such lines and make S
 * the current input.  Used when expanding aliases.  EXPAND is used to set
 * the value of expand_next_token when the string is popped, so that the
 * word after the alias in the original line is handled correctly when the
 * alias expands to multiple words.  TOKEN is the token that was expanded
 * into S; it is saved and used to prevent infinite recursive expansion.
 */
static void
push_string (s, expand, token)
     char *s;
     int expand;
     char *token;
{
  STRING_SAVER *temp = (STRING_SAVER *) xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
  temp->next = pushed_string_list;
  pushed_string_list = temp;

  save_expansion (token);

  shell_input_line = s;
  shell_input_line_size = strlen (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
  expand_next_token = 0;
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
  expand_next_token = pushed_string_list->expand_alias;

  t = pushed_string_list;
  pushed_string_list = pushed_string_list->next;
  free((char *)t);
}

static void
free_string_list ()
{
  register STRING_SAVER *t = pushed_string_list, *t1;

  while (t)
    {
      t1 = t->next;
      FREE (t->saved_line);
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

/* This is a stack to save the values of all tokens for which alias
   expansion has been performed during the current call to read_token ().
   It is used to prevent alias expansion loops:

      alias foo=bar
      alias bar=baz
      alias baz=foo

   Ideally this would be taken care of by push and pop string, but because
   of when strings are popped the stack will not contain the correct
   strings to test against.  (The popping is done in shell_getc, so that when
   the current string is exhausted, shell_getc can simply pop that string off
   the stack, restore the previous string, and continue with the character
   following the token whose expansion was originally pushed on the stack.)

   What we really want is a record of all tokens that have been expanded for
   aliases during the `current' call to read_token().  This does that, at the
   cost of being somewhat special-purpose (OK, OK vile and unclean). */

typedef struct _exp_saver {
      struct _exp_saver *next;
      char *saved_token;
} EXPANSION_SAVER;

EXPANSION_SAVER *expanded_token_stack = (EXPANSION_SAVER *)NULL;

static void
save_expansion (s)
     char *s;
{
  EXPANSION_SAVER *t;

  t = (EXPANSION_SAVER *) xmalloc (sizeof (EXPANSION_SAVER));
  t->saved_token = savestring (s);
  t->next = expanded_token_stack;
  expanded_token_stack = t;
}

/* Return 1 if TOKEN has already been expanded in the current `stack' of
   expansions.  If it has been expanded already, it will appear as the value
   of saved_token for some entry in the stack of expansions created for the
   current token being expanded. */
static int
token_has_been_expanded (token)
     char *token;
{
  register EXPANSION_SAVER *t = expanded_token_stack;

  while (t)
    {
      if (STREQ (token, t->saved_token))
	return (1);
      t = t->next;
    }
  return (0);
}

static void
free_expansion_stack ()
{
  register EXPANSION_SAVER *t = expanded_token_stack, *t1;

  while (t)
    {
      t1 = t->next;
      free (t->saved_token);
      free (t);
      t = t1;
    }
  expanded_token_stack = (EXPANSION_SAVER *)NULL;
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
	  if (indx == 0)
	    return ((char *)NULL);
	  c = '\n';
	}

      /* `+2' in case the final character in the buffer is a newline. */
      if (indx + 2 > buffer_size)
	if (!buffer_size)
	  line_buffer = xmalloc (buffer_size = 128);
	else
	  line_buffer = xrealloc (line_buffer, buffer_size += 128);

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
  { "{", '{' },
  { "}", '}' },
  { "!", BANG },
  { (char *)NULL, 0}
};

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line.  This is called by read_token when the shell is
   processing normal command input. */
static int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  int c;

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
      register int i, l;

      restart_read_next_line:

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

	  if (i + 2 > shell_input_line_size)
	    shell_input_line =
	      xrealloc (shell_input_line, shell_input_line_size += 256);

	  if (c == EOF)
	    {
	      if (bash_input.type == st_stream)
		clearerr (stdin);

	      if (!i)
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
      if (interactive && shell_input_line && shell_input_line[0])
	{
	  char *expansions;

	  expansions = pre_process_line (shell_input_line, 1, 1);

	  free (shell_input_line);
	  shell_input_line = expansions;
	  shell_input_line_len = shell_input_line ?
				 strlen (shell_input_line) :
				 0;
	  if (!shell_input_line_len)
	    current_command_line_count--;

	  /* We have to force the xrealloc below because we don't know the
	     true allocated size of shell_input_line anymore. */
	  shell_input_line_size = shell_input_line_len;
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
	  l = shell_input_line_len;	/* was a call to strlen */

	  if (l + 3 > shell_input_line_size)
	    shell_input_line = xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  shell_input_line[l] = '\n';
	  shell_input_line[l + 1] = '\0';
	}
    }
  
  c = shell_input_line[shell_input_line_index];

  if (c)
    shell_input_line_index++;

  if (c == '\\' && remove_quoted_newline &&
      shell_input_line[shell_input_line_index] == '\n')
    {
	prompt_again ();
	goto restart_read_next_line;
    }

#if defined (ALIAS)
  /* If C is NULL, we have reached the end of the current input string.  If
     pushed_string_list is non-empty, it's time to pop to the previous string
     because we have fully consumed the result of the last alias expansion.
     Do it transparently; just return the next character of the string popped
     to. */
  if (!c && (pushed_string_list != (STRING_SAVER *)NULL))
    {
      pop_string ();
      c = shell_input_line[shell_input_line_index];
      if (c)
	shell_input_line_index++;
    }
#endif /* ALIAS */

  if (!c && shell_input_line_terminator == EOF)
    {
      if (shell_input_line_index != 0)
	return ('\n');
      else
	return (EOF);
    }

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

/* Discard input until CHARACTER is seen. */
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

/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
static char *token = (char *)NULL;

/* Current size of the token buffer. */
static int token_buffer_size = 0;

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

  parse_and_execute (savestring (command), "PROMPT_COMMAND", 0);

  last_shell_builtin = temp_last;
  this_shell_builtin = temp_this;
  last_command_exit_value = temp_exit_value;
  eof_encountered = temp_eof_encountered;

  bind_variable ("_", last_lastarg);
  FREE (last_lastarg);

  if (token_to_read == '\n')
    token_to_read = 0;
}

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */

yylex ()
{
  if (interactive && (!current_token || current_token == '\n'))
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

  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
reset_parser ()
{
  read_token (RESET);
}
  
/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int allow_esac_as_next = 0;

/* When non-zero, accept single '{' as a token itself. */
static int allow_open_brace = 0;

/* DELIMITERS is a stack of the nested delimiters that we have
   encountered so far. */
static char *delimiters = (char *)NULL;

/* Offset into the stack of delimiters. */
int delimiter_depth = 0;

/* How many slots are allocated to DELIMITERS. */
static int delimiter_space = 0;

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

/* Macro for accessing the top delimiter on the stack.  Returns the
   delimiter or zero if none. */
#define current_delimiter() \
  (delimiter_depth ? delimiters[delimiter_depth - 1] : 0)

#define push_delimiter(character) \
  do \
    { \
      if (delimiter_depth + 2 > delimiter_space) \
	delimiters = xrealloc \
	  (delimiters, (delimiter_space += 10) * sizeof (char)); \
      delimiters[delimiter_depth] = character; \
      delimiter_depth++; \
    } \
  while (0)

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_awaiting_satisfaction = 0;

#define command_token_position(token) \
  (((token) == ASSIGNMENT_WORD) || \
   ((token) != SEMI_SEMI && reserved_word_acceptable(token)))

#define assignment_acceptable(token) command_token_position(token) && \
					(in_case_pattern_list == 0)

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
	      if (in_case_pattern_list && (word_token_alist[i].token != ESAC)) \
		break; \
\
	      if (word_token_alist[i].token == ESAC) \
		in_case_pattern_list = 0; \
\
	      if (word_token_alist[i].token == '{') \
		open_brace_awaiting_satisfaction++; \
\
	      if (word_token_alist[i].token == '}' && open_brace_awaiting_satisfaction) \
		open_brace_awaiting_satisfaction--; \
\
	      return (word_token_alist[i].token); \
	    } \
      } \
  } while (0)

/* Read the next token.  Command can be READ (normal operation) or 
   RESET (to normalize state). */
static int
read_token (command)
     int command;
{
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */
  WORD_DESC *the_word;		/* The value for YYLVAL when a WORD is read. */

  if (token_buffer_size < TOKEN_DEFAULT_GROW_SIZE)
    {
      FREE (token);
      token = xmalloc (token_buffer_size = TOKEN_DEFAULT_GROW_SIZE);
    }

  if (command == RESET)
    {
      delimiter_depth = 0;	/* No delimiters found so far. */
      open_brace_awaiting_satisfaction = 0;
      in_case_pattern_list = 0;

#if defined (ALIAS)
      if (pushed_string_list)
	{
	  free_string_list ();
	  pushed_string_list = (STRING_SAVER *)NULL;
	}

      if (expanded_token_stack)
	{
	  free_expansion_stack ();
	  expanded_token_stack = (EXPANSION_SAVER *)NULL;
	}

      expand_next_token = 0;
#endif /* ALIAS */

      if (shell_input_line)
	{
	  free (shell_input_line);
	  shell_input_line = (char *)NULL;
	  shell_input_line_size = shell_input_line_index = 0;
	}
      last_read_token = '\n';
      token_to_read = '\n';
      return ('\n');
    }

  if (token_to_read)
    {
      int rt = token_to_read;
      token_to_read = 0;
      return (rt);
    }

#if defined (ALIAS)
  /* If we hit read_token () and there are no saved strings on the
     pushed_string_list, then we are no longer currently expanding a
     token.  This can't be done in pop_stream, because pop_stream
     may pop the stream before the current token has finished being
     completely expanded (consider what happens when we alias foo to foo,
     and then try to expand it). */
  if (!pushed_string_list && expanded_token_stack)
    {
      free_expansion_stack ();
      expanded_token_stack = (EXPANSION_SAVER *)NULL;
    }

  /* This is a place to jump back to once we have successfully expanded a
     token with an alias and pushed the string with push_string () */
 re_read_token:

#endif /* ALIAS */

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && whitespace (character));

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

      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here documents. */
      if (need_here_doc)
        gather_here_documents ();

#if defined (ALIAS)
      expand_next_token = 0;
#endif /* ALIAS */

      return ('\n');
    }

  if (character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	gather_here_documents ();

#if defined (ALIAS)
      expand_next_token = 0;
#endif /* ALIAS */

      return (character);
    }

  if (member (character, "()<>;&|"))
    {
#if defined (ALIAS)
      /* Turn off alias tokenization iff this character sequence would
	 not leave us ready to read a command. */
      if (character == '<' || character == '>')
	expand_next_token = 0;
#endif /* ALIAS */

      /* Please note that the shell does not allow whitespace to
	 appear in between tokens which are character pairs, such as
	 "<<" or ">>".  I believe this is the correct behaviour. */
      if (character == (peek_char = shell_getc (1)))
	{
	  switch (character)
	    {
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	    case '<':
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
	      in_case_pattern_list = 1;
#if defined (ALIAS)
	      expand_next_token = 0;
#endif /* ALIAS */
	      return (SEMI_SEMI);

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);
	    }
	}
      else
	{
	  if (peek_char == '&')
	    {
	      switch (character)
		{
		case '<': return (LESS_AND);
		case '>': return (GREATER_AND);
		}
	    }
	  if (character == '<' && peek_char == '>')
	    return (LESS_GREATER);
	  if (character == '>' && peek_char == '|')
	    return (GREATER_BAR);
	  if (peek_char == '>' && character == '&')
	    return (AND_GREATER);
	}
      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if (character == ')' &&
	  last_read_token == '(' && token_before_that == WORD)
	{
	  allow_open_brace = 1;
#if defined (ALIAS)
	  expand_next_token = 0;
#endif /* ALIAS */
	}

      if (in_case_pattern_list && (character == ')'))
	in_case_pattern_list = 0;

#if defined (PROCESS_SUBSTITUTION)
      /* Check for the constructs which introduce process substitution.
	 Shells running in `posix mode' don't do process substitution. */
      if (posixly_correct ||
	  (((character == '>' || character == '<') && peek_char == '(') == 0))
#endif /* PROCESS_SUBSTITUTION */
	return (character);
    }

  /* Hack <&- (close stdin) case. */
  if (character == '-')
    {
      switch (last_read_token)
	{
	case LESS_AND:
	case GREATER_AND:
	  return (character);
	}
    }
  
  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  {
    /* Index into the token that we are building. */
    int token_index = 0;

    /* ALL_DIGITS becomes zero when we see a non-digit. */
    int all_digits = digit (character);

    /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
    int dollar_present = 0;

    /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
    int quoted = 0;

    /* Non-zero means to ignore the value of the next character, and just
       to add it no matter what. */
    int pass_next_character = 0;

    /* Non-zero means parsing a dollar-paren construct.  It is the count of
       un-quoted closes we need to see. */
    int dollar_paren_level = 0;

    /* Non-zero means parsing a dollar-bracket construct ($[...]).  It is
       the count of un-quoted `]' characters we need to see. */
    int dollar_bracket_level = 0;

    /* Non-zero means parsing a `${' construct.  It is the count of
       un-quoted `}' we need to see. */
    int dollar_brace_level = 0;

    /* A level variable for parsing '${ ... }' constructs inside of double
       quotes. */
    int delimited_brace_level = 0;

    /* A boolean variable denoting whether or not we are currently parsing
       a double-quoted string embedded in a $( ) or ${ } construct. */
    int embedded_quoted_string = 0;

    /* Another level variable.  This one is for dollar_parens inside of
       double-quotes. */
    int delimited_paren_level = 0;

    /* The current delimiting character. */
    int cd;

    for (;;)
      {
	if (character == EOF)
	  goto got_token;

	if (pass_next_character)
	  {
	    pass_next_character = 0;
	    goto got_character;
	  }

	cd = current_delimiter ();

	if (cd && character == '\\' && cd != '\'')
	  {
	    peek_char = shell_getc (0);
	    if (peek_char != '\\')
	      shell_ungetc (peek_char);
	    else
	      {
		token[token_index++] = character;
		goto got_character;
	      }
	  }

	/* Handle backslashes.  Quote lots of things when not inside of
	   double-quotes, quote some things inside of double-quotes. */
	   
	if (character == '\\' && (!delimiter_depth || cd != '\''))
	  {
	    peek_char = shell_getc (0);

	    /* Backslash-newline is ignored in all cases excepting
	       when quoted with single quotes. */
	    if (peek_char == '\n')
	      {
		character = '\n';
		goto next_character;
	      }
	    else
	      {
		shell_ungetc (peek_char);

		/* If the next character is to be quoted, do it now. */
		if (!cd || cd == '`' ||
		    (cd == '"' && member (peek_char, slashify_in_quotes)))
		  {
		    pass_next_character++;
		    quoted = 1;
		    goto got_character;
		  }
	      }
	  }

	/* This is a hack, in its present form.  If a backquote substitution
	   appears within double quotes, everything within the backquotes
	   should be read as part of a single word.  Jesus.  Now I see why
	   Korn introduced the $() form. */
	if (delimiter_depth && (cd == '"') && (character == '`'))
	  {
	    push_delimiter (character);
	    goto got_character;
	  }

	cd = current_delimiter ();		/* XXX - may not need */
	if (delimiter_depth)
	  {
	    if (character == cd)
	      {
	      	/* If we see a double quote while parsing a double-quoted
		  $( ) or ${ }, and we have not seen ) or }, respectively,
	      	   note that we are in the middle of reading an embedded
		   quoted string. */
		if ((delimited_paren_level || delimited_brace_level) &&
		    (character == '"'))
		  {
		    embedded_quoted_string = !embedded_quoted_string;
		    goto got_character;
		  }
		
		delimiter_depth--;
		goto got_character;
	      }
	  }

	if (cd != '\'')
	  {
#if defined (PROCESS_SUBSTITUTION)
	    if (character == '$' || character == '<' || character == '>')
#else
	    if (character == '$')
#endif /* !PROCESS_SUBSTITUTION */
	      {
	      	/* If we're in the middle of parsing a $( ) or ${ }
	      	   construct with an embedded quoted string, don't
	      	   bother looking at this character any further. */
	      	if (embedded_quoted_string)
	      	  goto got_character;

		peek_char = shell_getc (1);
		shell_ungetc (peek_char);
		if (peek_char == '(')
		  {
		    if (!delimiter_depth)
		      dollar_paren_level++;
		    else
		      delimited_paren_level++;

		    pass_next_character++;
		    goto got_character;
		  }
		else if (peek_char == '[' && character == '$')
		  {
		    if (!delimiter_depth)
		      dollar_bracket_level++;

		    pass_next_character++;
		    goto got_character;
		  }
		/* This handles ${...} constructs. */
		else if (peek_char == '{' && character == '$')
		  {
		    if (!delimiter_depth)
		      dollar_brace_level++;
		    else
		      delimited_brace_level++;

		    pass_next_character++;
		    goto got_character;
		  }
	      }

	    /* If we are parsing a $() or $[] construct, we need to balance
	       parens and brackets inside the construct.  This whole function
	       could use a rewrite. */
	    if (character == '(' && !embedded_quoted_string)
	      {
		if (delimiter_depth && delimited_paren_level)
		  delimited_paren_level++;

		if (!delimiter_depth && dollar_paren_level)
		  dollar_paren_level++;
	      }

	    if (character == '[')
	      {
		if (!delimiter_depth && dollar_bracket_level)
		  dollar_bracket_level++;
	      }

	    if (character == '{' && !embedded_quoted_string)
	      {
	      	if (delimiter_depth && delimited_brace_level)
	      	  delimited_brace_level++;

	      	if (!delimiter_depth && dollar_brace_level)
	      	  dollar_brace_level++;
	      }

	    /* This code needs to take into account whether we are inside a
	       case statement pattern list, and whether this paren is supposed
	       to terminate it (hey, it could happen).  It's not as simple
	       as just using in_case_pattern_list, because we're not parsing
	       anything while we're reading a $( ) construct.  Maybe we
	       should move that whole mess into the yacc parser. */
	    if (character == ')' && !embedded_quoted_string)
	      {
		if (delimiter_depth && delimited_paren_level)
		  delimited_paren_level--;

		if (!delimiter_depth && dollar_paren_level)
		  {
		    dollar_paren_level--;
		    goto got_character;
		  }
	      }

	    if (character == ']')
	      {
		if (!delimiter_depth && dollar_bracket_level)
		  {
		    dollar_bracket_level--;
		    goto got_character;
		  }
	      }

	    if (character == '}' && !embedded_quoted_string)
	      {
		if (delimiter_depth && delimited_brace_level)
		  delimited_brace_level--;

		if (!delimiter_depth && dollar_brace_level)
		  {
		    dollar_brace_level--;
		    goto got_character;
		  }
	      }
	  }

	if (!dollar_paren_level && !dollar_bracket_level &&
	    !dollar_brace_level && !delimiter_depth &&
	    member (character, " \t\n;&()|<>"))
	  {
	    shell_ungetc (character);
	    goto got_token;
	  }
    
	if (!delimiter_depth)
	  {
	    if (character == '"' || character == '`' || character == '\'')
	      {
		push_delimiter (character);

		quoted = 1;
		goto got_character;
	      }
	  }

	if (all_digits)
	  all_digits = digit (character);
	if (character == '$')
	  dollar_present = 1;

      got_character:

	if (character == CTLESC || character == CTLNUL)
	  token[token_index++] = CTLESC;

	token[token_index++] = character;

	if (token_index == (token_buffer_size - 1))
	  {
	    token_buffer_size += TOKEN_DEFAULT_GROW_SIZE;
	    token = xrealloc (token, token_buffer_size);
	  }
	next_character:
	if (character == '\n' && interactive && bash_input.type != st_string)
	  prompt_again ();

	/* We want to remove quoted newlines (that is, a \<newline> pair)
	   unless we are within single quotes or pass_next_character is
	   set (the shell equivalent of literal-next). */
	character = shell_getc
	  ((current_delimiter () != '\'') && (!pass_next_character));
      }

  got_token:

    token[token_index] = '\0';
	
    if ((delimiter_depth || dollar_paren_level || dollar_bracket_level) &&
	character == EOF)
      {
	char reporter = '\0';

	if (!delimiter_depth)
	  {
	    if (dollar_paren_level)
	      reporter = ')';
	    else if (dollar_bracket_level)
	      reporter = ']';
	  }

	if (!reporter)
	  reporter = current_delimiter ();

	report_error ("unexpected EOF while looking for `%c'", reporter);
	return (-1);
      }

    if (all_digits)
      {
	/* Check to see what thing we should return.  If the last_read_token
	   is a `<', or a `&', or the character which ended this token is
	   a '>' or '<', then, and ONLY then, is this input token a NUMBER.
	   Otherwise, it is just a word, and should be returned as such. */

	if (character == '<' || character == '>' ||
	    last_read_token == LESS_AND || last_read_token == GREATER_AND)
	  {
	    yylval.number = atoi (token);
	    return (NUMBER);
	  }
      }

    /* Handle special case.  IN is recognized if the last token
       was WORD and the token before that was FOR or CASE. */
    if ((last_read_token == WORD) &&
#if defined (SELECT_COMMAND)
	((token_before_that == FOR) || (token_before_that == CASE) || (token_before_that == SELECT)) &&
#else
	((token_before_that == FOR) || (token_before_that == CASE)) &&
#endif
	(token[0] == 'i' && token[1] == 'n' && !token[2]))
      {
	if (token_before_that == CASE)
	  {
	    in_case_pattern_list = 1;
	    allow_esac_as_next++;
	  }
	return (IN);
      }

    /* Ditto for DO in the FOR case. */
#if defined (SELECT_COMMAND)
    if ((last_read_token == WORD) && ((token_before_that == FOR) || (token_before_that == SELECT)) &&
#else
    if ((last_read_token == WORD) && (token_before_that == FOR) &&
#endif
	(token[0] == 'd' && token[1] == 'o' && !token[2]))
      return (DO);

    /* Ditto for ESAC in the CASE case. 
       Specifically, this handles "case word in esac", which is a legal
       construct, certainly because someone will pass an empty arg to the
       case construct, and we don't want it to barf.  Of course, we should
       insist that the case construct has at least one pattern in it, but
       the designers disagree. */
    if (allow_esac_as_next)
      {
	allow_esac_as_next--;
	if (STREQ (token, "esac"))
	  {
	    in_case_pattern_list = 0;
	    return (ESAC);
	  }
      }

    /* Ditto for `{' in the FUNCTION case. */
    if (allow_open_brace)
      {
	allow_open_brace = 0;
	if (token[0] == '{' && !token[1])
	  {
	    open_brace_awaiting_satisfaction++;
	    return ('{');
	  }
      }

    if (posixly_correct)
      CHECK_FOR_RESERVED_WORD (token);

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
	 In a pattern list in a case statement (in_case_pattern_list). */
    if (interactive_shell && !quoted && !in_case_pattern_list &&
	(expand_next_token || command_token_position (last_read_token)))
      {
	char *alias_expand_word (), *expanded;

	if (expanded_token_stack && token_has_been_expanded (token))
	  goto no_expansion;

	expanded = alias_expand_word (token);
	if (expanded)
	  {
	    int len = strlen (expanded), expand_next;

	    /* Erase the current token. */
	    token_index = 0;

	    expand_next = (expanded[len - 1] == ' ') ||
			  (expanded[len - 1] == '\t');

	    push_string (expanded, expand_next, token);
	    goto re_read_token;
	  }
	else
	  /* This is an eligible token that does not have an expansion. */
no_expansion:
	  expand_next_token = 0;
      }
    else
      {
	expand_next_token = 0;
      }
#endif /* ALIAS */

    if (!posixly_correct)
      CHECK_FOR_RESERVED_WORD (token);

    /* What if we are attempting to satisfy an open-brace grouper? */
    if (open_brace_awaiting_satisfaction && token[0] == '}' && !token[1])
      {
	open_brace_awaiting_satisfaction--;
	return ('}');
      }

    the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
    the_word->word = xmalloc (1 + token_index);
    strcpy (the_word->word, token);
    the_word->dollar_present = dollar_present;
    the_word->quoted = quoted;
    the_word->assignment = assignment (token);

    yylval.word = the_word;
    result = WORD;

    /* A word is an assignment if it appears at the beginning of a
       simple command, or after another assignment word.  This is
       context-dependent, so it cannot be handled in the grammar. */
    if (assignment_acceptable (last_read_token) && the_word->assignment)
      result = ASSIGNMENT_WORD;

    if (last_read_token == FUNCTION)
      allow_open_brace = 1;
  }
  return (result);
}

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (token)
     int token;
{
#if 0
  if (member (token, "\n;()|&{") ||
#else
  if (token == '\n' || token == ';' || token == '(' || token == ')' ||
      token == '|' || token == '&' || token == '{' ||
#endif
      token == '}' ||			/* XXX */
      token == AND_AND ||
      token == BANG ||
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
  for (i = 0; word_token_alist[i].word != (char *)NULL; i++)
    if (STREQ (token, word_token_alist[i].word))
      return i;
  return -1;
}

#if defined (READLINE)
/* Called after each time readline is called.  This insures that whatever
   the new prompt string is gets propagated to readline's local prompt
   variable. */
static void
reset_readline_prompt ()
{
  if (prompt_string_pointer)
    {
      char *temp_prompt;

      temp_prompt = *prompt_string_pointer
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

#if defined (HISTORY)
/* A list of tokens which can be followed by newlines, but not by
   semi-colons.  When concatenating multiple lines of history, the
   newline separator for such tokens is replaced with a space. */
static int no_semi_successors[] = {
  '\n', '{', '(', ')', ';', '&', '|',
  CASE, DO, ELSE, IF, IN, SEMI_SEMI, THEN, UNTIL, WHILE, AND_AND, OR_OR,
  0
};

/* If we are not within a delimited expression, try to be smart
   about which separators can be semi-colons and which must be
   newlines. */
char *
history_delimiting_chars ()
{
  if (!delimiter_depth)
    {
      register int i;

      for (i = 0; no_semi_successors[i]; i++)
	{
	  if (token_before_that == no_semi_successors[i])
	    return (" ");
	}
      return ("; ");
    }
  else
    return ("\n");
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

  temp_prompt = (*prompt_string_pointer)
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
   
	\t	the time
	\d	the date
	\n	CRLF
	\s	the name of the shell
	\w	the current working directory
	\W	the last element of PWD
	\u	your username
	\h	the hostname
	\#	the command number of this command
	\!	the history number of this command
	\$	a $ or a # if you are root
	\<octal> character code in octal
	\\	a backslash
*/
#define PROMPT_GROWTH 50
char *
decode_prompt_string (string)
     char *string;
{
  int result_size = PROMPT_GROWTH;
  int result_index = 0;
  char *result;
  int c;
  char *temp = (char *)NULL;
  WORD_LIST *list;

#if defined (PROMPT_STRING_DECODE)

  result = xmalloc (PROMPT_GROWTH);
  result[0] = 0;

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
	      {
		char octal_string[4];
		int n;

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
	      }
	  
	    case 't':
	    case 'd':
	      /* Make the current time/date into a string. */
	      {
		time_t the_time = time (0);
		char *ttemp = ctime (&the_time);
		temp = savestring (ttemp);

		if (c == 't')
		  {
		    strcpy (temp, temp + 11);
		    temp[8] = '\0';
		  }
		else
		  temp[10] = '\0';

		goto add_string;
	      }

	    case 'n':
	      if (!no_line_editing)
		temp = savestring ("\r\n");
	      else
		temp = savestring ("\n");
	      goto add_string;

	    case 's':
	      {
		temp = base_pathname (shell_name);
		temp = savestring (temp);
		goto add_string;
	      }
	
	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more effecient. */
#define EFFICIENT
#ifdef EFFICIENT
		char *polite_directory_format (), t_string[MAXPATHLEN];

		temp = get_string_value ("PWD");

		if (!temp)
		  getwd (t_string);
		else
		  strcpy (t_string, temp);
#else
		getwd (t_string);
#endif	/* EFFICIENT */

		if (c == 'W')
		  {
		    char *dir = (char *)strrchr (t_string, '/');
		    if (dir && dir != t_string)
		      strcpy (t_string, dir + 1);
		    temp = savestring (t_string);
		  }
		else
		  temp = savestring (polite_directory_format (t_string));
		goto add_string;
	      }
      
	    case 'u':
	      {
		temp = savestring (current_user.user_name);
		goto add_string;
	      }

	    case 'h':
	      {
		char *t_string;

		temp = savestring (current_host_name);
		if (t_string = (char *)strchr (temp, '.'))
		  *t_string = '\0';
		goto add_string;
	      }

	    case '#':
	      {
		temp = itos (current_command_number);
		goto add_string;
	      }

	    case '!':
	      {
#if !defined (HISTORY)
		temp = savestring ("1");
#else /* HISTORY */
		temp = itos (history_number ());
#endif /* HISTORY */
		goto add_string;
	      }

	    case '$':
	      temp = savestring (geteuid () == 0 ? "#" : "$");
	      goto add_string;

#if defined (READLINE)
	    case '[':
	    case ']':
	      temp = xmalloc(3);
	      temp[0] = '\001';
	      temp[1] = (c == '[') ? RL_PROMPT_START_IGNORE : RL_PROMPT_END_IGNORE;
	      temp[2] = '\0';
	      goto add_string;
#endif

	    case '\\':
	      temp = savestring ("\\");
	      goto add_string;

	    default:
	      temp = savestring ("\\ ");
	      temp[1] = c;

	    add_string:
	      if (c)
		string++;
	      result =
		sub_append_string (temp, result, &result_index, &result_size);
	      temp = (char *)NULL; /* Free ()'ed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  while (3 + result_index > result_size)
	    result = xrealloc (result, result_size += PROMPT_GROWTH);

	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }
#else /* !PROMPT_STRING_DECODE */
  result = savestring (string);
#endif /* !PROMPT_STRING_DECODE */

  /* Perform variable and parameter expansion and command substitution on
     the prompt string. */
  list = expand_string_unsplit (result, 1);
  free (result);
  result = string_list (list);
  dispose_words (list);

  return (result);
}

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
yyerror ()
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
static void
report_syntax_error (message)
     char *message;
{
  if (message)
    {
      if (!interactive)
	{
	  char *name = bash_input.name ? bash_input.name : "stdin";
	  report_error ("%s: line %d: `%s'", name, line_number, message);
	}
      else
	{
	  if (EOF_Reached)
	    EOF_Reached = 0;
	  report_error ("%s", message);
	}

      last_command_exit_value = EX_USAGE;
      return;
    }

  if (shell_input_line && *shell_input_line)
    {
      char *t = shell_input_line;
      register int i = shell_input_line_index;
      int token_end = 0;

      if (!t[i] && i)
	i--;

      while (i && (t[i] == ' ' || t[i] == '\t' || t[i] == '\n'))
	i--;

      if (i)
	token_end = i + 1;

      while (i && !member (t[i], " \n\t;|&"))
	i--;

      while (i != token_end && member (t[i], " \t\n"))
	i++;

      if (token_end)
	{
	  char *error_token;
	  error_token = xmalloc (1 + (token_end - i));
	  strncpy (error_token, t + i, token_end - i);
	  error_token[token_end - i] = '\0';

	  report_error ("syntax error near unexpected token `%s'", error_token);
	  free (error_token);
	}
      else if ((i == 0) && (token_end == 0))	/* a 1-character token */
	{
	  char etoken[2];
	  etoken[0] = t[i];
	  etoken[1] = '\0';

	  report_error ("syntax error near unexpected token `%s'", etoken);
	}

      if (!interactive)
	{
	  char *temp = savestring (shell_input_line);
	  char *name = bash_input.name ? bash_input.name : "stdin";
	  int l = strlen (temp);

	  while (l && temp[l - 1] == '\n')
	    temp[--l] = '\0';

	  report_error ("%s: line %d: `%s'", name, line_number, temp);
	  free (temp);
	}
    }
  else
    {
      char *name, *msg;
      if (!interactive)
	name = bash_input.name ? bash_input.name : "stdin";
      if (EOF_Reached)
	msg = "syntax error: unexpected end of file";
      else
	msg = "syntax error";
      if (!interactive)
	report_error ("%s: line %d: %s", name, line_number, msg);
      else
	{
	  /* This file uses EOF_Reached only for error reporting
	     when the shell is interactive.  Other mechanisms are 
	     used to decide whether or not to exit. */
	  EOF_Reached = 0;
	  report_error (msg);
	}
    }
  last_command_exit_value = EX_USAGE;
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command. */
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
