#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
#line 22 "/usr/homes/chet/src/bash/src/parse.y"
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
#  ifndef _MINIX
#    include <sys/param.h>
#  endif
#  include <time.h>
#  include "maxpath.h"
#endif /* PROMPT_STRING_DECODE */

#define RE_READ_TOKEN	-99
#define NO_EXPANSION	-100

#define YYDEBUG 0

#if defined (EXTENDED_GLOB)
extern int extended_glob;
#endif

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
extern int dump_translatable_strings, dump_po_strings;
extern Function *last_shell_builtin, *this_shell_builtin;
#if defined (BUFFERED_INPUT)
extern int bash_input_fd_changed;
#endif

extern int errno;
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
#if defined (COND_COMMAND)
static COMMAND *parse_cond_command ();
#endif
static int read_token_word ();
static void discard_parser_constructs ();

static void report_syntax_error ();
static void handle_eof_input_unit ();
static void prompt_again ();
#if 0
static void reset_readline_prompt ();
#endif
static void print_prompt ();

#if defined (HISTORY)
char *history_delimiting_chars ();
#endif

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

/* The line number in a script at which an arithmetic for command starts. */
static int arith_for_lineno;

static REDIRECTEE redir;
#line 190 "/usr/homes/chet/src/bash/src/parse.y"
typedef union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
} YYSTYPE;
#line 194 "y.tab.c"
#define YYERRCODE 256
#define IF 257
#define THEN 258
#define ELSE 259
#define ELIF 260
#define FI 261
#define CASE 262
#define ESAC 263
#define FOR 264
#define SELECT 265
#define WHILE 266
#define UNTIL 267
#define DO 268
#define DONE 269
#define FUNCTION 270
#define COND_START 271
#define COND_END 272
#define COND_ERROR 273
#define IN 274
#define BANG 275
#define TIME 276
#define TIMEOPT 277
#define WORD 278
#define ASSIGNMENT_WORD 279
#define NUMBER 280
#define ARITH_CMD 281
#define ARITH_FOR_EXPRS 282
#define COND_CMD 283
#define AND_AND 284
#define OR_OR 285
#define GREATER_GREATER 286
#define LESS_LESS 287
#define LESS_AND 288
#define GREATER_AND 289
#define SEMI_SEMI 290
#define LESS_LESS_MINUS 291
#define AND_GREATER 292
#define LESS_GREATER 293
#define GREATER_BAR 294
#define yacc_EOF 295
const short yylhs[] = {                                        -1,
    0,    0,    0,    0,   27,   27,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   26,   26,   26,   25,   25,   10,   10,
    1,    1,    1,    1,   11,   11,   11,   11,   11,   11,
   11,   11,   11,   11,   11,   12,   12,   12,   12,   12,
   12,   18,   18,   18,   18,   13,   13,   13,   13,   13,
   13,   14,   14,   14,   19,   19,   19,   20,   20,   23,
   21,   21,   21,   15,   16,   17,   22,   22,   22,   31,
   31,   29,   29,   29,   29,   30,   30,   28,   28,    4,
    7,    7,    5,    5,    5,    6,    6,    6,    6,    6,
    6,   34,   34,   34,   33,   33,    8,    8,    8,    9,
    9,    9,    9,    9,    3,    3,    3,    3,    3,    2,
    2,   32,   32,
};
const short yylen[] = {                                         2,
    2,    1,    2,    1,    1,    2,    2,    2,    3,    3,
    2,    3,    2,    3,    2,    3,    2,    3,    2,    3,
    2,    3,    2,    3,    2,    3,    2,    3,    2,    3,
    2,    2,    3,    1,    1,    1,    1,    2,    1,    2,
    1,    1,    2,    1,    1,    1,    5,    5,    1,    1,
    1,    1,    1,    1,    1,    6,    6,    7,    7,   10,
   10,    7,    7,    5,    5,    6,    6,    7,    7,   10,
   10,    6,    7,    6,    5,    6,    4,    1,    2,    3,
    5,    7,    6,    3,    1,    3,    4,    6,    5,    1,
    2,    4,    4,    5,    5,    2,    3,    1,    3,    2,
    1,    2,    3,    3,    3,    4,    4,    4,    4,    4,
    1,    1,    1,    1,    0,    2,    1,    2,    2,    4,
    4,    3,    3,    1,    1,    2,    2,    3,    3,    4,
    1,    1,    2,
};
const short yydefred[] = {                                      0,
    0,  115,    0,    0,    0,  115,  115,    0,    0,    0,
    0,    0,   35,    0,   85,    0,    0,    0,    0,    0,
    0,    0,    0,    2,    4,    0,    0,  115,  115,    0,
  131,    0,  124,    0,    0,    0,    0,   45,   49,   46,
   52,   53,   54,   55,   44,   50,   51,   36,   39,    0,
    3,  101,    0,    0,  115,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  133,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   11,   13,   19,   15,   27,
   21,   17,   25,   23,   29,   31,   32,    7,    8,    0,
    0,  115,    1,  115,  115,    0,    0,   34,   40,   37,
    0,    0,    0,  115,  116,  111,  100,    0,    0,  115,
    0,  115,  113,  112,  114,  115,  115,  115,    0,  115,
  115,    0,    0,   86,    0,  115,   12,   14,   20,   16,
   28,   22,   18,   26,   24,   30,   33,    9,   10,   84,
   80,    0,    0,    0,    0,    0,   38,    0,    0,  115,
  115,  115,  115,  115,  115,    0,  115,    0,  115,    0,
    0,    0,    0,  115,    0,  115,    0,    0,  115,    0,
   77,    0,    0,  120,  121,    0,    0,  115,  115,   81,
    0,    0,    0,    0,    0,    0,    0,  115,    0,    0,
  115,  115,    0,    5,    0,    0,   64,   65,  115,  115,
  115,  115,    0,    0,    0,    0,   47,   48,    0,    0,
   75,    0,    0,   83,  106,  107,    0,    0,    0,   96,
    0,    0,   74,   72,   98,    0,    0,    0,    0,   56,
    6,  115,   57,    0,    0,    0,    0,   66,    0,  115,
   67,   76,   82,  115,  115,  115,  115,   97,   73,    0,
    0,  115,   58,   59,    0,   62,   63,   68,   69,    0,
    0,    0,    0,    0,  115,   99,   92,    0,  115,  115,
  115,  115,  115,   89,   94,    0,    0,    0,    0,    0,
   88,   60,   61,   70,   71,
};
const short yydgoto[] = {                                      30,
   31,   32,  106,   52,  107,  108,   53,   34,  145,   36,
   37,   38,   39,   40,   41,   42,   43,   44,   45,  171,
   46,  181,   47,   48,  101,   49,  195,  227,  187,  188,
  189,   50,   54,  117,
};
const short yysindex[] = {                                    -10,
    2,    0, -258, -255, -246,    0,    0, -237, -249,  846,
 -226,   15,    0,  123,    0, -211, -203,  -44,  -43, -199,
 -198, -194, -193,    0,    0, -192, -191,    0,    0,    0,
    0,  -41,    0,   78,   23,  956,  975,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  721,
    0,    0, -169,   71,    0,   35,   -6,   39, -177, -172,
   61, -168,  -41,  933,    0,   62, -176, -173,  -30,  -27,
 -166, -163, -162, -160, -159,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  -17,
   69,    0,    0,    0,    0,  813,  813,    0,    0,    0,
  975,  933,  -41,    0,    0,    0,    0,   19,   -7,    0,
   66,    0,    0,    0,    0,    0,    0,    0,   99,    0,
    0,   79,  305,    0,  -41,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  104,   71,   71, -247, -247,    0,  -41, -195,    0,
    0,    0,    0,    0,    0,    4,    0, -156,    0, -145,
    1,    7,   12,    0, -156,    0, -144, -140,    0,  975,
    0,  305,  -41,    0,    0,  813,  813,    0,    0,    0,
 -138,   71,   71,   71,   71,   71, -158,    0, -129,   -4,
    0,    0, -132,    0,   -5,   16,    0,    0,    0,    0,
    0,    0, -126,   71,   -5,   20,    0,    0,  305,  975,
    0, -115, -111,    0,    0,    0, -240, -240, -240,    0,
 -142,   32,    0,    0,    0, -128,  -34, -120,   26,    0,
    0,    0,    0, -114,   36, -107,   38,    0,   19,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  -25,
 -113,    0,    0,    0,   29,    0,    0,    0,    0,   37,
 -201,   71,   71,   71,    0,    0,    0,   71,    0,    0,
    0,    0,    0,    0,    0,   71, -100,   45,  -98,   48,
    0,    0,    0,    0,    0,
};
const short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  888,   33,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  450,    0,    0,  164,  118,  162,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  126,    0,  126,    0,    0,
  216,    0,  462,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  165,  166,    0,    0,    0,
  174,    0,  495,    0,    0,    0,    0,  436,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  507,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   11,   18,    0,  540,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  403,
    0,    0,  553,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  573,  662,  765,  -85,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  416,
    0,    0,    0,    0,    0,    0,  587,  599,  611,    0,
  -84,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -81,    0,    0,    0,    0,    0,    0, -250,    0,    0,
    0,    0,    0,    0,    0, -244,    0,    0,    0,    0,
    0,    0,    0,    0,    0,
};
const short yygindex[] = {                                      0,
    0,   -2,   10, -133,    0,  340,  798,    0,   24,    0,
 -112,    0,    0,    0,    0,    0,    0,    0,    0, -146,
    0,  -80,    0,  -28,   21,  152,   25,  -33,    8,    0,
    0,  182, 1196, -170,
};
#define YYTABLESIZE 1468
const short yytable[] = {                                      24,
   80,   83,  105,  114,  114,  105,  252,   63,  100,   33,
  170,   51,   93,  105,  131,  265,  105,  134,   95,   55,
  122,  105,   56,   35,  232,  211,   57,  123,  154,   29,
  203,   58,  206,   62,  240,  226,   94,   95,  105,   93,
   61,  105,   34,  150,  151,   95,  105,  103,  122,   27,
   65,   26,  113,  113,   66,  123,  152,  273,  179,  170,
   96,  125,  242,  178,  179,  180,   76,  236,  237,  122,
   34,  226,  147,   34,   77,  105,  123,  153,   84,   85,
  105,   97,   92,   86,   87,   88,   89,   93,  104,  251,
  120,   34,   34,  110,   34,  121,  170,  118,  251,  148,
  122,  127,  126,  124,  128,   33,   33,  140,  105,  141,
   29,  135,   28,  105,  136,  137,  116,  138,  139,  169,
  146,  194,  214,  197,  207,  198,  192,   41,  208,  200,
   27,  220,   26,  223,  202,  115,  230,  279,  280,  173,
  233,  100,  238,   29,  241,  243,  244,  248,  253,  225,
  254,  270,   33,   33,  256,   41,   34,   34,   41,  272,
  257,  258,  259,   27,  266,   26,  174,  175,  282,  283,
  284,   42,  285,  117,  118,  119,   41,   90,   91,   87,
  274,  147,   75,   43,   74,   33,   33,   99,  159,  205,
  210,   64,  250,   28,    0,  221,    0,    0,    0,   42,
  146,    0,   42,    0,    0,    0,    0,    0,    0,    0,
    0,   43,    0,    0,   43,    0,    0,    0,    0,    0,
   42,  166,    0,    0,    0,  115,   28,    0,    0,    0,
    0,    0,   43,   78,   81,   79,   82,    0,    0,    0,
    0,   41,   41,    0,    0,    1,    2,  129,  115,  130,
  132,    3,  133,    4,    5,    6,    7,    0,  224,    8,
    9,  112,    0,    0,   10,   11,  155,   12,   13,   14,
   15,  191,  231,  225,  199,   16,   17,   18,   19,  201,
   20,   21,   22,   23,   25,   42,   42,    0,  115,  115,
   34,   34,   34,   34,  249,   34,  269,   43,   43,    0,
   34,   34,  150,  151,  271,    0,   94,   95,    0,  225,
   34,   34,   34,    0,  105,    0,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,    2,    0,    0,
    0,    0,    3,  157,    4,    5,    6,    7,  115,  158,
    8,    9,    0,    0,   29,   10,   11,    0,   12,   13,
   14,   15,    0,    0,    0,    0,   16,   17,   18,   19,
    2,   20,   21,   22,   23,    3,  164,    4,    5,    6,
    7,    0,  165,    8,    9,   41,   41,   41,   41,    0,
   41,   12,   13,   14,   15,   41,   41,    0,    0,   16,
   17,   18,   19,  115,   20,   21,   22,   23,    0,  115,
    0,   41,   41,    0,    0,    0,    0,   41,   67,   68,
   69,   70,   78,   71,    0,   72,   73,    0,    0,   42,
   42,   42,   42,    0,   42,   79,    0,   28,    0,   42,
   42,   43,   43,   43,   43,    0,   43,    0,    0,    0,
   78,   43,   43,   78,    0,   42,   42,    0,    0,    0,
    0,   42,    0,   79,    0,    0,   79,   43,   43,  125,
    0,   78,    0,   43,    0,    0,    0,    0,    0,    0,
    0,  126,  115,    0,   79,    0,  102,  115,    0,  115,
  115,  115,  115,    0,    0,    0,  115,  125,    0,    0,
  125,    0,    0,    0,    0,    0,  115,    0,    0,  126,
    0,    0,  126,    0,  127,    0,    0,    0,  125,    0,
    0,    0,    0,    0,    0,    0,  129,    0,    0,    0,
  126,  215,  216,  217,  218,  219,   78,   78,    0,    0,
    0,    0,  127,    0,    0,  127,    0,    0,    0,   79,
   79,    0,    0,  239,  129,    0,    0,  129,    0,  128,
    0,    0,    0,  127,    0,    0,    0,    0,    0,    0,
  102,    2,  130,    0,    0,  129,    3,    0,    4,    5,
    6,    7,    0,    0,  125,    9,    0,  128,    0,    0,
  128,    0,    0,    0,    0,   15,  126,    0,    0,    0,
  130,    0,    0,  130,    0,    0,  108,    0,  128,    0,
    0,  217,  218,  219,    0,    0,    0,    0,  109,    0,
    0,  130,    0,  104,    0,    0,    0,    0,    0,  127,
  110,    0,    0,    0,  108,    0,    0,  108,    0,    0,
    0,  129,    0,    0,    0,    0,  109,    0,    0,  109,
    0,    0,    0,    0,    0,  108,    0,    0,  110,    0,
    0,  110,    0,    0,    0,    0,    0,  109,    0,    0,
   78,   78,   78,   78,  128,   78,    0,    0,    0,  110,
   78,   78,    0,   79,   79,   79,   79,  130,   79,    0,
    0,    0,    0,   79,   79,    0,   78,   78,    0,    0,
    0,    0,   78,  102,  102,  102,  102,  104,  102,   79,
   79,    0,  105,  102,  102,   79,    0,  125,  125,  125,
  125,  108,  125,    0,    0,    0,    0,  125,  125,  126,
  126,  126,  126,  109,  126,  102,    0,    0,    0,  126,
  126,    0,    0,  125,  125,  110,    0,    0,    0,  125,
    0,    0,    0,    0,    0,  126,  126,    0,    0,    0,
    0,  126,  127,  127,  127,  127,    0,  127,    0,    0,
   29,    0,  127,  127,  129,  129,  129,  129,    0,  129,
    0,    0,    0,    0,  129,  129,    0,    0,  127,  127,
   27,    0,   26,    0,  127,    0,  105,    0,    0,    0,
  129,  129,    0,    0,    0,    0,  129,  128,  128,  128,
  128,    0,  128,   59,   60,  103,    0,  128,  128,    0,
  130,  130,  130,  130,    0,  130,    0,    0,    0,    0,
  130,  130,    0,  128,  128,   90,   91,    0,    0,  128,
  104,  104,  104,  104,    0,  104,  130,  130,    0,    0,
  104,  104,  130,   28,  108,  108,  108,  108,    0,  108,
    0,    0,   29,    0,  108,  108,  109,  109,  109,  109,
    0,  109,  104,    0,    0,    0,  109,  109,  110,  110,
  110,  110,   27,  110,   26,    0,  108,    0,  110,  110,
    0,    0,    0,    0,    0,   29,    0,    0,  109,  103,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  110,  149,    0,    0,    0,   27,    0,   26,    0,  160,
    0,    0,    0,  161,    0,    0,    0,  167,  168,  105,
  105,  105,  105,    0,  105,    0,    0,  132,    0,  105,
  105,    0,    0,    0,    0,   28,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  132,    0,  132,
    0,  105,    0,    0,  193,    0,  196,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   28,    0,
    0,    0,   29,    0,    0,  212,  213,    2,    0,    0,
    0,    0,    3,    0,    4,    5,    6,    7,  228,  229,
    8,    9,   27,    0,   26,  102,  234,  235,   12,   13,
   14,   15,    0,    0,    0,    0,   16,   17,   18,   19,
  132,   20,   21,   22,   23,   27,    0,   26,    0,    0,
    0,    0,  103,  103,  103,  103,    0,  103,    0,    0,
    0,    0,  103,  103,   27,    0,   26,    0,    0,    0,
    0,  261,    0,    0,    0,    0,    0,    0,    0,  267,
    0,    0,    0,    0,  103,   28,    0,    0,    0,    0,
    0,    0,  275,    0,    0,    0,  277,  278,    0,    2,
  281,    0,    0,    0,    3,    0,    4,    5,    6,    7,
    0,    0,    8,    9,    0,    0,    0,   10,   11,    0,
   12,   13,   14,   15,    0,    0,    0,    0,   16,   17,
   18,   19,    2,   20,   21,   22,   23,    3,    0,    4,
    5,    6,    7,    0,    0,    8,    9,    0,    0,    0,
    0,   11,    0,   12,   13,   14,   15,    0,    0,    0,
    0,   16,   17,   18,   19,    0,   20,   21,   22,   23,
    0,    0,    0,    0,  132,    0,    0,    0,    0,  132,
    0,  132,  132,  132,  132,    0,    0,  132,  132,    0,
    0,    0,  132,    0,    0,  132,  132,  132,  132,    0,
    0,    0,    0,  132,  132,  132,  132,    0,  132,  132,
  132,  132,    0,    0,    0,    0,    0,    0,    0,    2,
    0,    0,    0,    0,    3,    0,    4,    5,    6,    7,
    0,    0,    8,    9,    0,    0,    0,    0,    0,    0,
   12,   13,   14,   15,    0,    0,    0,    0,   16,   17,
   18,   19,    0,   20,   21,   22,   23,    0,    0,    0,
    0,    0,    0,   98,   13,   14,    0,    0,    0,    0,
    0,   16,   17,   18,   19,    0,   20,   21,   22,   23,
  109,  111,    0,  119,   14,    0,  123,    0,    0,    0,
   16,   17,   18,   19,    0,   20,   21,   22,   23,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  142,    0,  143,
  144,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  156,    0,    0,    0,    0,
    0,    0,  162,  163,    0,    0,    0,    0,    0,    0,
    0,  172,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  182,  183,  184,  185,  186,
  190,    0,    0,    0,    0,    0,    0,    0,    0,  204,
    0,  204,    0,    0,  209,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  222,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  204,  204,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  255,    0,    0,
    0,    0,    0,    0,    0,  260,    0,    0,    0,    0,
  262,  263,  264,    0,    0,    0,    0,  268,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  276,    0,    0,    0,    0,    0,  204,  204,
};
const short yycheck[] = {                                      10,
   45,   45,   10,   10,   10,   10,   41,   10,   37,    0,
  123,   10,  263,   10,   45,   41,   10,   45,  263,  278,
   10,   10,  278,    0,  195,  172,  282,   10,   10,   40,
  164,  278,  166,  283,  205,   40,  284,  285,   10,  290,
  278,   10,   10,  284,  285,  290,   10,   50,   38,   60,
  277,   62,   59,   59,   40,   38,   38,  259,  260,  172,
   38,   64,  209,  259,  260,  261,  278,  201,  202,   59,
   38,   40,  101,   41,  278,   10,   59,   59,  278,  278,
   10,   59,  124,  278,  278,  278,  278,   10,  258,  124,
  268,   59,   60,   59,   62,  268,  209,   59,  124,  102,
   40,  278,   41,  272,  278,   96,   97,  125,   10,   41,
   40,  278,  123,   10,  278,  278,  123,  278,  278,   41,
   97,  278,  261,  269,  269,  125,  123,   10,  269,  123,
   60,  290,   62,  263,  123,   10,  269,  271,  272,  142,
  125,  170,  269,   40,  125,  261,  258,  290,  269,  278,
  125,  123,  143,  144,  269,   38,  124,  125,   41,  123,
  125,  269,  125,   60,  278,   62,  143,  144,  269,  125,
  269,   10,  125,   10,   10,   10,   59,  263,  263,  261,
  261,  210,   60,   10,   62,  176,  177,   36,  123,  165,
  170,   10,  226,  123,   -1,  188,   -1,   -1,   -1,   38,
  177,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   38,   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,
   59,  123,   -1,   -1,   -1,   10,  123,   -1,   -1,   -1,
   -1,   -1,   59,  278,  278,  280,  280,   -1,   -1,   -1,
   -1,  124,  125,   -1,   -1,  256,  257,  278,  123,  280,
  278,  262,  280,  264,  265,  266,  267,   -1,  263,  270,
  271,  268,   -1,   -1,  275,  276,  274,  278,  279,  280,
  281,  268,  278,  278,  268,  286,  287,  288,  289,  268,
  291,  292,  293,  294,  295,  124,  125,   -1,  295,  295,
  258,  259,  260,  261,  263,  263,  268,  124,  125,   -1,
  268,  269,  284,  285,  268,   -1,  284,  285,   -1,  278,
  278,  279,  280,   -1,   10,   -1,  284,  285,  286,  287,
  288,  289,  290,  291,  292,  293,  294,  257,   -1,   -1,
   -1,   -1,  262,  268,  264,  265,  266,  267,  123,  274,
  270,  271,   -1,   -1,   40,  275,  276,   -1,  278,  279,
  280,  281,   -1,   -1,   -1,   -1,  286,  287,  288,  289,
  257,  291,  292,  293,  294,  262,  268,  264,  265,  266,
  267,   -1,  274,  270,  271,  258,  259,  260,  261,   -1,
  263,  278,  279,  280,  281,  268,  269,   -1,   -1,  286,
  287,  288,  289,  268,  291,  292,  293,  294,   -1,  274,
   -1,  284,  285,   -1,   -1,   -1,   -1,  290,  286,  287,
  288,  289,   10,  291,   -1,  293,  294,   -1,   -1,  258,
  259,  260,  261,   -1,  263,   10,   -1,  123,   -1,  268,
  269,  258,  259,  260,  261,   -1,  263,   -1,   -1,   -1,
   38,  268,  269,   41,   -1,  284,  285,   -1,   -1,   -1,
   -1,  290,   -1,   38,   -1,   -1,   41,  284,  285,   10,
   -1,   59,   -1,  290,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   10,  257,   -1,   59,   -1,   41,  262,   -1,  264,
  265,  266,  267,   -1,   -1,   -1,  271,   38,   -1,   -1,
   41,   -1,   -1,   -1,   -1,   -1,  281,   -1,   -1,   38,
   -1,   -1,   41,   -1,   10,   -1,   -1,   -1,   59,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,   -1,   -1,
   59,  182,  183,  184,  185,  186,  124,  125,   -1,   -1,
   -1,   -1,   38,   -1,   -1,   41,   -1,   -1,   -1,  124,
  125,   -1,   -1,  204,   38,   -1,   -1,   41,   -1,   10,
   -1,   -1,   -1,   59,   -1,   -1,   -1,   -1,   -1,   -1,
  125,  257,   10,   -1,   -1,   59,  262,   -1,  264,  265,
  266,  267,   -1,   -1,  125,  271,   -1,   38,   -1,   -1,
   41,   -1,   -1,   -1,   -1,  281,  125,   -1,   -1,   -1,
   38,   -1,   -1,   41,   -1,   -1,   10,   -1,   59,   -1,
   -1,  262,  263,  264,   -1,   -1,   -1,   -1,   10,   -1,
   -1,   59,   -1,   41,   -1,   -1,   -1,   -1,   -1,  125,
   10,   -1,   -1,   -1,   38,   -1,   -1,   41,   -1,   -1,
   -1,  125,   -1,   -1,   -1,   -1,   38,   -1,   -1,   41,
   -1,   -1,   -1,   -1,   -1,   59,   -1,   -1,   38,   -1,
   -1,   41,   -1,   -1,   -1,   -1,   -1,   59,   -1,   -1,
  258,  259,  260,  261,  125,  263,   -1,   -1,   -1,   59,
  268,  269,   -1,  258,  259,  260,  261,  125,  263,   -1,
   -1,   -1,   -1,  268,  269,   -1,  284,  285,   -1,   -1,
   -1,   -1,  290,  258,  259,  260,  261,  125,  263,  284,
  285,   -1,   41,  268,  269,  290,   -1,  258,  259,  260,
  261,  125,  263,   -1,   -1,   -1,   -1,  268,  269,  258,
  259,  260,  261,  125,  263,  290,   -1,   -1,   -1,  268,
  269,   -1,   -1,  284,  285,  125,   -1,   -1,   -1,  290,
   -1,   -1,   -1,   -1,   -1,  284,  285,   -1,   -1,   -1,
   -1,  290,  258,  259,  260,  261,   -1,  263,   -1,   -1,
   40,   -1,  268,  269,  258,  259,  260,  261,   -1,  263,
   -1,   -1,   -1,   -1,  268,  269,   -1,   -1,  284,  285,
   60,   -1,   62,   -1,  290,   -1,  125,   -1,   -1,   -1,
  284,  285,   -1,   -1,   -1,   -1,  290,  258,  259,  260,
  261,   -1,  263,    6,    7,   41,   -1,  268,  269,   -1,
  258,  259,  260,  261,   -1,  263,   -1,   -1,   -1,   -1,
  268,  269,   -1,  284,  285,   28,   29,   -1,   -1,  290,
  258,  259,  260,  261,   -1,  263,  284,  285,   -1,   -1,
  268,  269,  290,  123,  258,  259,  260,  261,   -1,  263,
   -1,   -1,   40,   -1,  268,  269,  258,  259,  260,  261,
   -1,  263,  290,   -1,   -1,   -1,  268,  269,  258,  259,
  260,  261,   60,  263,   62,   -1,  290,   -1,  268,  269,
   -1,   -1,   -1,   -1,   -1,   40,   -1,   -1,  290,  125,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  290,  104,   -1,   -1,   -1,   60,   -1,   62,   -1,  112,
   -1,   -1,   -1,  116,   -1,   -1,   -1,  120,  121,  258,
  259,  260,  261,   -1,  263,   -1,   -1,   40,   -1,  268,
  269,   -1,   -1,   -1,   -1,  123,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   60,   -1,   62,
   -1,  290,   -1,   -1,  157,   -1,  159,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  123,   -1,
   -1,   -1,   40,   -1,   -1,  178,  179,  257,   -1,   -1,
   -1,   -1,  262,   -1,  264,  265,  266,  267,  191,  192,
  270,  271,   60,   -1,   62,  275,  199,  200,  278,  279,
  280,  281,   -1,   -1,   -1,   -1,  286,  287,  288,  289,
  123,  291,  292,  293,  294,   60,   -1,   62,   -1,   -1,
   -1,   -1,  258,  259,  260,  261,   -1,  263,   -1,   -1,
   -1,   -1,  268,  269,   60,   -1,   62,   -1,   -1,   -1,
   -1,  244,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  252,
   -1,   -1,   -1,   -1,  290,  123,   -1,   -1,   -1,   -1,
   -1,   -1,  265,   -1,   -1,   -1,  269,  270,   -1,  257,
  273,   -1,   -1,   -1,  262,   -1,  264,  265,  266,  267,
   -1,   -1,  270,  271,   -1,   -1,   -1,  275,  276,   -1,
  278,  279,  280,  281,   -1,   -1,   -1,   -1,  286,  287,
  288,  289,  257,  291,  292,  293,  294,  262,   -1,  264,
  265,  266,  267,   -1,   -1,  270,  271,   -1,   -1,   -1,
   -1,  276,   -1,  278,  279,  280,  281,   -1,   -1,   -1,
   -1,  286,  287,  288,  289,   -1,  291,  292,  293,  294,
   -1,   -1,   -1,   -1,  257,   -1,   -1,   -1,   -1,  262,
   -1,  264,  265,  266,  267,   -1,   -1,  270,  271,   -1,
   -1,   -1,  275,   -1,   -1,  278,  279,  280,  281,   -1,
   -1,   -1,   -1,  286,  287,  288,  289,   -1,  291,  292,
  293,  294,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,
   -1,   -1,   -1,   -1,  262,   -1,  264,  265,  266,  267,
   -1,   -1,  270,  271,   -1,   -1,   -1,   -1,   -1,   -1,
  278,  279,  280,  281,   -1,   -1,   -1,   -1,  286,  287,
  288,  289,   -1,  291,  292,  293,  294,   -1,   -1,   -1,
   -1,   -1,   -1,  278,  279,  280,   -1,   -1,   -1,   -1,
   -1,  286,  287,  288,  289,   -1,  291,  292,  293,  294,
   55,   56,   -1,   58,  280,   -1,   61,   -1,   -1,   -1,
  286,  287,  288,  289,   -1,  291,  292,  293,  294,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   92,   -1,   94,
   95,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  110,   -1,   -1,   -1,   -1,
   -1,   -1,  117,  118,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  126,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  150,  151,  152,  153,  154,
  155,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  164,
   -1,  166,   -1,   -1,  169,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  188,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  201,  202,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  232,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  240,   -1,   -1,   -1,   -1,
  245,  246,  247,   -1,   -1,   -1,   -1,  252,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  265,   -1,   -1,   -1,   -1,   -1,  271,  272,
};
#define YYFINAL 30
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 295
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"'&'",0,"'('","')'",0,0,0,"'-'",0,0,0,0,0,0,0,0,0,0,0,0,0,"';'",
"'<'",0,"'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'","'|'","'}'",0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"IF","THEN","ELSE","ELIF","FI","CASE","ESAC","FOR","SELECT",
"WHILE","UNTIL","DO","DONE","FUNCTION","COND_START","COND_END","COND_ERROR",
"IN","BANG","TIME","TIMEOPT","WORD","ASSIGNMENT_WORD","NUMBER","ARITH_CMD",
"ARITH_FOR_EXPRS","COND_CMD","AND_AND","OR_OR","GREATER_GREATER","LESS_LESS",
"LESS_AND","GREATER_AND","SEMI_SEMI","LESS_LESS_MINUS","AND_GREATER",
"LESS_GREATER","GREATER_BAR","yacc_EOF",
};
const char * const yyrule[] = {
"$accept : inputunit",
"inputunit : simple_list '\\n'",
"inputunit : '\\n'",
"inputunit : error '\\n'",
"inputunit : yacc_EOF",
"word_list : WORD",
"word_list : word_list WORD",
"redirection : '>' WORD",
"redirection : '<' WORD",
"redirection : NUMBER '>' WORD",
"redirection : NUMBER '<' WORD",
"redirection : GREATER_GREATER WORD",
"redirection : NUMBER GREATER_GREATER WORD",
"redirection : LESS_LESS WORD",
"redirection : NUMBER LESS_LESS WORD",
"redirection : LESS_AND NUMBER",
"redirection : NUMBER LESS_AND NUMBER",
"redirection : GREATER_AND NUMBER",
"redirection : NUMBER GREATER_AND NUMBER",
"redirection : LESS_AND WORD",
"redirection : NUMBER LESS_AND WORD",
"redirection : GREATER_AND WORD",
"redirection : NUMBER GREATER_AND WORD",
"redirection : LESS_LESS_MINUS WORD",
"redirection : NUMBER LESS_LESS_MINUS WORD",
"redirection : GREATER_AND '-'",
"redirection : NUMBER GREATER_AND '-'",
"redirection : LESS_AND '-'",
"redirection : NUMBER LESS_AND '-'",
"redirection : AND_GREATER WORD",
"redirection : NUMBER LESS_GREATER WORD",
"redirection : LESS_GREATER WORD",
"redirection : GREATER_BAR WORD",
"redirection : NUMBER GREATER_BAR WORD",
"simple_command_element : WORD",
"simple_command_element : ASSIGNMENT_WORD",
"simple_command_element : redirection",
"redirection_list : redirection",
"redirection_list : redirection_list redirection",
"simple_command : simple_command_element",
"simple_command : simple_command simple_command_element",
"command : simple_command",
"command : shell_command",
"command : shell_command redirection_list",
"command : function_def",
"shell_command : for_command",
"shell_command : case_command",
"shell_command : WHILE compound_list DO compound_list DONE",
"shell_command : UNTIL compound_list DO compound_list DONE",
"shell_command : select_command",
"shell_command : if_command",
"shell_command : subshell",
"shell_command : group_command",
"shell_command : arith_command",
"shell_command : cond_command",
"shell_command : arith_for_command",
"for_command : FOR WORD newline_list DO compound_list DONE",
"for_command : FOR WORD newline_list '{' compound_list '}'",
"for_command : FOR WORD ';' newline_list DO compound_list DONE",
"for_command : FOR WORD ';' newline_list '{' compound_list '}'",
"for_command : FOR WORD newline_list IN word_list list_terminator newline_list DO compound_list DONE",
"for_command : FOR WORD newline_list IN word_list list_terminator newline_list '{' compound_list '}'",
"arith_for_command : FOR ARITH_FOR_EXPRS list_terminator newline_list DO compound_list DONE",
"arith_for_command : FOR ARITH_FOR_EXPRS list_terminator newline_list '{' compound_list '}'",
"arith_for_command : FOR ARITH_FOR_EXPRS DO compound_list DONE",
"arith_for_command : FOR ARITH_FOR_EXPRS '{' compound_list '}'",
"select_command : SELECT WORD newline_list DO list DONE",
"select_command : SELECT WORD newline_list '{' list '}'",
"select_command : SELECT WORD ';' newline_list DO list DONE",
"select_command : SELECT WORD ';' newline_list '{' list '}'",
"select_command : SELECT WORD newline_list IN word_list list_terminator newline_list DO list DONE",
"select_command : SELECT WORD newline_list IN word_list list_terminator newline_list '{' list '}'",
"case_command : CASE WORD newline_list IN newline_list ESAC",
"case_command : CASE WORD newline_list IN case_clause_sequence newline_list ESAC",
"case_command : CASE WORD newline_list IN case_clause ESAC",
"function_def : WORD '(' ')' newline_list function_body",
"function_def : FUNCTION WORD '(' ')' newline_list function_body",
"function_def : FUNCTION WORD newline_list function_body",
"function_body : shell_command",
"function_body : shell_command redirection_list",
"subshell : '(' compound_list ')'",
"if_command : IF compound_list THEN compound_list FI",
"if_command : IF compound_list THEN compound_list ELSE compound_list FI",
"if_command : IF compound_list THEN compound_list elif_clause FI",
"group_command : '{' compound_list '}'",
"arith_command : ARITH_CMD",
"cond_command : COND_START COND_CMD COND_END",
"elif_clause : ELIF compound_list THEN compound_list",
"elif_clause : ELIF compound_list THEN compound_list ELSE compound_list",
"elif_clause : ELIF compound_list THEN compound_list elif_clause",
"case_clause : pattern_list",
"case_clause : case_clause_sequence pattern_list",
"pattern_list : newline_list pattern ')' compound_list",
"pattern_list : newline_list pattern ')' newline_list",
"pattern_list : newline_list '(' pattern ')' compound_list",
"pattern_list : newline_list '(' pattern ')' newline_list",
"case_clause_sequence : pattern_list SEMI_SEMI",
"case_clause_sequence : case_clause_sequence pattern_list SEMI_SEMI",
"pattern : WORD",
"pattern : pattern '|' WORD",
"list : newline_list list0",
"compound_list : list",
"compound_list : newline_list list1",
"list0 : list1 '\\n' newline_list",
"list0 : list1 '&' newline_list",
"list0 : list1 ';' newline_list",
"list1 : list1 AND_AND newline_list list1",
"list1 : list1 OR_OR newline_list list1",
"list1 : list1 '&' newline_list list1",
"list1 : list1 ';' newline_list list1",
"list1 : list1 '\\n' newline_list list1",
"list1 : pipeline_command",
"list_terminator : '\\n'",
"list_terminator : ';'",
"list_terminator : yacc_EOF",
"newline_list :",
"newline_list : newline_list '\\n'",
"simple_list : simple_list1",
"simple_list : simple_list1 '&'",
"simple_list : simple_list1 ';'",
"simple_list1 : simple_list1 AND_AND newline_list simple_list1",
"simple_list1 : simple_list1 OR_OR newline_list simple_list1",
"simple_list1 : simple_list1 '&' simple_list1",
"simple_list1 : simple_list1 ';' simple_list1",
"simple_list1 : pipeline_command",
"pipeline_command : pipeline",
"pipeline_command : BANG pipeline",
"pipeline_command : timespec pipeline",
"pipeline_command : timespec BANG pipeline",
"pipeline_command : BANG timespec pipeline",
"pipeline : pipeline '|' newline_list pipeline",
"pipeline : command",
"timespec : TIME",
"timespec : TIME TIMEOPT",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
#line 815 "/usr/homes/chet/src/bash/src/parse.y"

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
	give_terminal_to (shell_pgrp, 0);
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
  STRING_SAVER *temp = (STRING_SAVER *) xmalloc (sizeof (STRING_SAVER));

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

/* XXX - we should also have an alist with strings for other tokens, so we
	 can give more descriptive error messages.  Look at y.tab.h for the
	 other tokens. */

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

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
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
#endif /* ALIAS || DPAREN_ARITHMETIC */

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
  else
    eol_ungetc_lookahead = c;
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

  /* We allow a `do' after a for ((...)) without an intervening
     list_terminator */
  if (last_read_token == ARITH_FOR_EXPRS && token[0] == 'd' && token[1] == 'o' && !token[2])
    return (DO);
  if (last_read_token == ARITH_FOR_EXPRS && token[0] == '{' && token[1] == '\0')	/* } */
    {
      open_brace_count++;
      return ('{');			/* } */
    }

  if (open_brace_count && reserved_word_acceptable (last_read_token) && token[0] == '}' && !token[1])
    {
      open_brace_count--;		/* { */
      return ('}');
    }

#if defined (COMMAND_TIMING)
  /* Handle -p after `time'. */
  if (last_read_token == TIME && token[0] == '-' && token[1] == 'p' && !token[2])
    return (TIMEOPT);
#endif

#if defined (COMMAND_TIMING)
  if (STREQ (token, "time") && ((parser_state & PST_CASEPAT) == 0) && time_command_acceptable ())
    return (TIME);
#endif /* COMMAND_TIMING */

#if defined (COND_COMMAND) /* [[ */
  if ((parser_state & PST_CONDEXPR) && token[0] == ']' && token[1] == ']' && token[2] == '\0')
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
	  if (EOF_Reached && cond_token != COND_ERROR)		/* [[ */
	    parser_error (cond_lineno, "unexpected EOF while looking for `]]'");
	  else if (cond_token != COND_ERROR)
	    parser_error (cond_lineno, "syntax error in conditional expression");
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

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
	    case '(':		/* ) */
#  if defined (ARITH_FOR_COMMAND)
	      if (last_read_token == FOR)
		{
		  int cmdtyp, len;
		  char *wval, *wv2;
		  WORD_DESC *wd;

		  arith_for_lineno = line_number;
		  cmdtyp = parse_arith_cmd (&wval);
		  if (cmdtyp == 1)
		    {
		      /* parse_arith_cmd adds quotes at the beginning and end
			 of the string it returns; we need to take those out. */
		      len = strlen (wval);
		      wv2 = xmalloc (len);
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
#  endif
#  if defined (DPAREN_ARITHMETIC)
	      if (reserved_word_acceptable (last_read_token))
		{
		  int cmdtyp, sline;
		  char *wval;
		  WORD_DESC *wd;

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
		      return (character);
		    }
		  else			/* ERROR */
		    return -1;
		}
	      break;
#  endif
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

  ret = xmalloc (retsize = 64);
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
#if 1
      /* handle nested ${...} specially. */
      else if (open != close && was_dollar && open == '{' && ch == open) /* } */
	count++;
#endif
      else if (((flags & P_FIRSTCLOSE) == 0) && ch == open)		/* nested begin */
	count++;

      /* Add this character. */
      RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
      ret[retind++] = ch;

      if (open == '\'')			/* '' inside grouping construct */
	{
	  if ((flags & P_ALLOWESC) && ch == '\\')
	    pass_next_character++;
	  continue;
	}

      if (ch == '\\')			/* backslashes */
	pass_next_character++;

      if (open != close)		/* a grouping construct */
	{
	  if (shellquote (ch))
	    {
	      /* '', ``, or "" inside $(...) or other grouping construct. */
	      push_delimiter (dstack, ch);
	      if (was_dollar && ch == '\'')	/* $'...' inside group */
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, P_ALLOWESC);
	      else
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, 0);
	      pop_delimiter (dstack);
	      if (nestret == &matched_pair_error)
		{
		  free (ret);
		  return &matched_pair_error;
		}
	      if (was_dollar && ch == '\'')
		{
		  /* Translate $'...' here. */
		  ttrans = ansiexpand (nestret, 0, nestlen - 1, &ttranslen);
		  free (nestret);
		  nestret = sh_single_quote (ttrans);
		  free (ttrans);
		  nestlen = strlen (nestret);
		  retind -= 2;		/* back up before the $' */
		}
	      else if (was_dollar && ch == '"')
		{
		  /* Locale expand $"..." here. */
		  ttrans = localeexpand (nestret, 0, nestlen - 1, start_lineno, &ttranslen);
		  free (nestret);
		  nestret = xmalloc (ttranslen + 3);
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
      was_dollar = (ch == '$');
    }

  ret[retind] = '\0';
  if (lenp)
    *lenp = retind;
  return ret;
}

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
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

  token = xmalloc (ttoklen + 4);

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
#endif /* DPAREN_ARITHMETIC || ARITH_FOR_COMMAND */

#if defined (COND_COMMAND)
static COND_COM *cond_term ();
static COND_COM *cond_and ();
static COND_COM *cond_or ();
static COND_COM *cond_expr ();

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
  all_digits = isdigit (character);
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
		  (cd == '"' && (sh_syntaxtab[peek_char] & CBSDQUOTE)))
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

#ifdef EXTENDED_GLOB
      /* Parse a ksh-style extended pattern matching specification. */
      if (extended_glob && PATTERN_CHAR (character))
	{
	  peek_char = shell_getc (1);
	  if (peek_char == '(')		/* ) */
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
	      dollar_present = all_digits = 0;
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
	  if (peek_char == '(' ||
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
	      all_digits = 0;
	      goto next_character;
	    }
	  /* This handles $'...' and $"..." new-style quoted strings. */
	  else if (character == '$' && (peek_char == '\'' || peek_char == '"'))
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
		  ttok = xmalloc (ttranslen + 3);
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
	      all_digits = 0;
	      goto next_character;
	    }
	  /* This could eventually be extended to recognize all of the
	     shell's single-character parameter expansions, and set flags.*/
	  else if (character == '$' && peek_char == '$')
	    {
	      ttok = xmalloc (3);
	      ttok[0] = ttok[1] = '$';
	      ttok[2] = '\0';
	      RESIZE_MALLOCED_BUFFER (token, token_index, 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttok);
	      token_index += 2;
	      dollar_present = 1;
	      all_digits = 0;
	      FREE (ttok);
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}

#if defined (ARRAY_VARS)
      /* Identify possible compound array variable assignment. */
      else if (character == '=' && token_index > 0)
	{
	  peek_char = shell_getc (1);
	  if (peek_char == '(')		/* ) */
	    {
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      if (ttok[0] == '(')	/* ) */
		{
		  FREE (ttok);
		  return -1;
		}
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

      all_digits &= isdigit (character);
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
      t = ansicstr (temp, tlen, 0, (int *)NULL, lenp);
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

/* Change a bash string into a string suitable for inclusion in a `po' file.
   This backslash-escapes `"' and `\' and changes newlines into \\\n"\n". */
static char *
mk_msgstr (string, foundnlp)
     char *string;
     int *foundnlp;
{
  register int c, len;
  char *result, *r, *s;

  for (len = 0, s = string; s && *s; s++)
    {
      len++;
      if (*s == '"' || *s == '\\')
	len++;
      else if (*s == '\n')
	len += 5;
    }
  
  r = result = xmalloc (len + 3);
  *r++ = '"';

  for (s = string; s && (c = *s); s++)
    {
      if (c == '\n')	/* <NL> -> \n"<NL>" */
	{
	  *r++ = '\\';
	  *r++ = 'n';
	  *r++ = '"';
	  *r++ = '\n';
	  *r++ = '"';
	  if (foundnlp)
	    *foundnlp = 1;
	  continue;
	}
      if (c == '"' || c == '\\')
	*r++ = '\\';
      *r++ = c;
    }

  *r++ = '"';
  *r++ = '\0';

  return result;
}

/* $"..." -- Translate the portion of STRING between START and END
   according to current locale using gettext (if available) and return
   the result.  The caller will take care of leaving the quotes intact.
   The string will be left without the leading `$' by the caller.
   If translation is performed, the translated string will be double-quoted
   by the caller.  The length of the translated string is returned in LENP,
   if non-null. */
static char *
localeexpand (string, start, end, lineno, lenp)
     char *string;
     int start, end, lineno, *lenp;
{
  int len, tlen, foundnl;
  char *temp, *t, *t2;

  temp = xmalloc (end - start + 1);
  for (tlen = 0, len = start; len < end; )
    temp[tlen++] = string[len++];
  temp[tlen] = '\0';

  /* If we're just dumping translatable strings, don't do anything with the
     string itself, but if we're dumping in `po' file format, convert it into a form more palatable to gettext(3)
     and friends by quoting `"' and `\' with backslashes and converting <NL>
     into `\n"<NL>"'.  If we find a newline in TEMP, we first output a
     `msgid ""' line and then the translated string; otherwise we output the
     `msgid' and translated string all on one line. */
  if (dump_translatable_strings)
    {
      if (dump_po_strings)
	{
	  foundnl = 0;
	  t = mk_msgstr (temp, &foundnl);
	  t2 = foundnl ? "\"\"\n" : "";

	  printf ("#: %s:%d\nmsgid %s%s\nmsgstr \"\"\n",
		  (bash_input.name ? bash_input.name : "stdin"), lineno, t2, t);
	  free (t);
	}
      else
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
	\e	escape (ascii 033)
	\d	the date in Day Mon Date format
	\h	the hostname up to the first `.'
	\H	the hostname
	\j	the number of active jobs
	\l	the basename of the shell's tty device name
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

	    case 'r':
	      temp = xmalloc (2);
	      temp[0] = '\r';
	      temp[1] = '\0';
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
		     second argument of Q_DOUBLE_QUOTE if we use this
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
	      t = temp = xmalloc (3);
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
      list = expand_prompt_string (result, Q_DOUBLE_QUOTES);
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
	    msg = substring (t, i, token_end);
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

static WORD_LIST parse_string_error;

/* Take a string and run it through the shell parser, returning the
   resultant word list.  Used by compound array assignment. */
WORD_LIST *
parse_string_to_word_list (s, whom)
     char *s, *whom;
{
  WORD_LIST *wl;
  int tok, orig_line_number, orig_input_terminator;
  int orig_line_count;
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

  push_stream (1);
  last_read_token = '\n';
  current_command_line_count = 0;

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
	  yyerror ();	/* does the right thing */
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
#line 4199 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 241 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = yyvsp[-1].command;
			  eof_encountered = 0;
			  discard_parser_constructs (0);
			  YYACCEPT;
			}
break;
case 2:
#line 250 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			}
break;
case 3:
#line 257 "/usr/homes/chet/src/bash/src/parse.y"
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
			}
break;
case 4:
#line 272 "/usr/homes/chet/src/bash/src/parse.y"
{
			  /* Case of EOF seen by itself.  Do ignoreeof or
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			}
break;
case 5:
#line 282 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); }
break;
case 6:
#line 284 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-1].word_list); }
break;
case 7:
#line 288 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_direction, redir);
			}
break;
case 8:
#line 293 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_direction, redir);
			}
break;
case 9:
#line 298 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_direction, redir);
			}
break;
case 10:
#line 303 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_direction, redir);
			}
break;
case 11:
#line 308 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_appending_to, redir);
			}
break;
case 12:
#line 313 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_appending_to, redir);
			}
break;
case 13:
#line 318 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
break;
case 14:
#line 324 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
break;
case 15:
#line 330 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (0, r_duplicating_input, redir);
			}
break;
case 16:
#line 335 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input, redir);
			}
break;
case 17:
#line 340 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (1, r_duplicating_output, redir);
			}
break;
case 18:
#line 345 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = yyvsp[0].number;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output, redir);
			}
break;
case 19:
#line 350 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_duplicating_input_word, redir);
			}
break;
case 20:
#line 355 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_input_word, redir);
			}
break;
case 21:
#line 360 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_duplicating_output_word, redir);
			}
break;
case 22:
#line 365 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_duplicating_output_word, redir);
			}
break;
case 23:
#line 370 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (0, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
break;
case 24:
#line 377 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection
			    (yyvsp[-2].number, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = yyval.redirect;
			}
break;
case 25:
#line 384 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (1, r_close_this, redir);
			}
break;
case 26:
#line 389 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			}
break;
case 27:
#line 394 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (0, r_close_this, redir);
			}
break;
case 28:
#line 399 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.dest = 0L;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_close_this, redir);
			}
break;
case 29:
#line 404 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_err_and_out, redir);
			}
break;
case 30:
#line 409 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_input_output, redir);
			}
break;
case 31:
#line 414 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (0, r_input_output, redir);
			}
break;
case 32:
#line 419 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (1, r_output_force, redir);
			}
break;
case 33:
#line 424 "/usr/homes/chet/src/bash/src/parse.y"
{
			  redir.filename = yyvsp[0].word;
			  yyval.redirect = make_redirection (yyvsp[-2].number, r_output_force, redir);
			}
break;
case 34:
#line 431 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; }
break;
case 35:
#line 433 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.element.word = yyvsp[0].word; yyval.element.redirect = 0; }
break;
case 36:
#line 435 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.element.redirect = yyvsp[0].redirect; yyval.element.word = 0; }
break;
case 37:
#line 439 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.redirect = yyvsp[0].redirect;
			}
break;
case 38:
#line 443 "/usr/homes/chet/src/bash/src/parse.y"
{
			  register REDIRECT *t;

			  for (t = yyvsp[-1].redirect; t->next; t = t->next)
			    ;
			  t->next = yyvsp[0].redirect;
			  yyval.redirect = yyvsp[-1].redirect;
			}
break;
case 39:
#line 454 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, (COMMAND *)NULL); }
break;
case 40:
#line 456 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_simple_command (yyvsp[0].element, yyvsp[-1].command); }
break;
case 41:
#line 460 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = clean_simple_command (yyvsp[0].command); }
break;
case 42:
#line 462 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 43:
#line 464 "/usr/homes/chet/src/bash/src/parse.y"
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
case 44:
#line 480 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 45:
#line 484 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 46:
#line 486 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 47:
#line 488 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_while_command (yyvsp[-3].command, yyvsp[-1].command); }
break;
case 48:
#line 490 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_until_command (yyvsp[-3].command, yyvsp[-1].command); }
break;
case 49:
#line 492 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 50:
#line 494 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 51:
#line 496 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 52:
#line 498 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 53:
#line 500 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 54:
#line 502 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 55:
#line 504 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 56:
#line 508 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); }
break;
case 57:
#line 510 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command); }
break;
case 58:
#line 512 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); }
break;
case 59:
#line 514 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command); }
break;
case 60:
#line 516 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); }
break;
case 61:
#line 518 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_for_command (yyvsp[-8].word, REVERSE_LIST (yyvsp[-5].word_list, WORD_LIST *), yyvsp[-1].command); }
break;
case 62:
#line 522 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-5].word_list, yyvsp[-1].command, arith_for_lineno); }
break;
case 63:
#line 524 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-5].word_list, yyvsp[-1].command, arith_for_lineno); }
break;
case 64:
#line 526 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-3].word_list, yyvsp[-1].command, arith_for_lineno); }
break;
case 65:
#line 528 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_for_command (yyvsp[-3].word_list, yyvsp[-1].command, arith_for_lineno); }
break;
case 66:
#line 532 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
break;
case 67:
#line 536 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-4].word, add_string_to_list ("$@", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
break;
case 68:
#line 540 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
break;
case 69:
#line 544 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yyvsp[-1].command);
			}
break;
case 70:
#line 548 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command);
			}
break;
case 71:
#line 552 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_select_command (yyvsp[-8].word, (WORD_LIST *)reverse_list (yyvsp[-5].word_list), yyvsp[-1].command);
			}
break;
case 72:
#line 558 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, (PATTERN_LIST *)NULL); }
break;
case 73:
#line 560 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_case_command (yyvsp[-5].word, yyvsp[-2].pattern); }
break;
case 74:
#line 562 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_case_command (yyvsp[-4].word, yyvsp[-1].pattern); }
break;
case 75:
#line 566 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command, function_dstart, function_bstart); }
break;
case 76:
#line 569 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_function_def (yyvsp[-4].word, yyvsp[0].command, function_dstart, function_bstart); }
break;
case 77:
#line 572 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_function_def (yyvsp[-2].word, yyvsp[0].command, function_dstart, function_bstart); }
break;
case 78:
#line 577 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 79:
#line 579 "/usr/homes/chet/src/bash/src/parse.y"
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
case 80:
#line 610 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = make_subshell_command (yyvsp[-1].command);
			  yyval.command->flags |= CMD_WANT_SUBSHELL;
			}
break;
case 81:
#line 617 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, (COMMAND *)NULL); }
break;
case 82:
#line 619 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-5].command, yyvsp[-3].command, yyvsp[-1].command); }
break;
case 83:
#line 621 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[-1].command); }
break;
case 84:
#line 626 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_group_command (yyvsp[-1].command); }
break;
case 85:
#line 630 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_arith_command (yyvsp[0].word_list); }
break;
case 86:
#line 634 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[-1].command; }
break;
case 87:
#line 638 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-2].command, yyvsp[0].command, (COMMAND *)NULL); }
break;
case 88:
#line 640 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-4].command, yyvsp[-2].command, yyvsp[0].command); }
break;
case 89:
#line 642 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = make_if_command (yyvsp[-3].command, yyvsp[-1].command, yyvsp[0].command); }
break;
case 91:
#line 647 "/usr/homes/chet/src/bash/src/parse.y"
{ yyvsp[0].pattern->next = yyvsp[-1].pattern; yyval.pattern = yyvsp[0].pattern; }
break;
case 92:
#line 651 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); }
break;
case 93:
#line 653 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); }
break;
case 94:
#line 655 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, yyvsp[0].command); }
break;
case 95:
#line 657 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.pattern = make_pattern_list (yyvsp[-2].word_list, (COMMAND *)NULL); }
break;
case 97:
#line 662 "/usr/homes/chet/src/bash/src/parse.y"
{ yyvsp[-1].pattern->next = yyvsp[-2].pattern; yyval.pattern = yyvsp[-1].pattern; }
break;
case 98:
#line 666 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, (WORD_LIST *)NULL); }
break;
case 99:
#line 668 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.word_list = make_word_list (yyvsp[0].word, yyvsp[-2].word_list); }
break;
case 100:
#line 677 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			 }
break;
case 102:
#line 686 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[0].command;
			}
break;
case 104:
#line 693 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, (COMMAND *)NULL, '&');
			}
break;
case 106:
#line 704 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); }
break;
case 107:
#line 706 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); }
break;
case 108:
#line 708 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-3].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-3].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '&');
			}
break;
case 109:
#line 715 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); }
break;
case 110:
#line 717 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, ';'); }
break;
case 111:
#line 719 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 117:
#line 738 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			}
break;
case 118:
#line 744 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-1].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yyvsp[-1].command, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents ();
			}
break;
case 119:
#line 753 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyval.command = yyvsp[-1].command;
			  if (need_here_doc)
			    gather_here_documents ();
			}
break;
case 120:
#line 761 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, AND_AND); }
break;
case 121:
#line 763 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, OR_OR); }
break;
case 122:
#line 765 "/usr/homes/chet/src/bash/src/parse.y"
{
			  if (yyvsp[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yyvsp[-2].command, yyvsp[0].command, '&');
			  else
			    yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, '&');
			}
break;
case 123:
#line 772 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-2].command, yyvsp[0].command, ';'); }
break;
case 124:
#line 775 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 125:
#line 779 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 126:
#line 781 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			}
break;
case 127:
#line 786 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-1].number;
			  yyval.command = yyvsp[0].command;
			}
break;
case 128:
#line 791 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-2].number|CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			}
break;
case 129:
#line 796 "/usr/homes/chet/src/bash/src/parse.y"
{
			  yyvsp[0].command->flags |= yyvsp[-1].number|CMD_INVERT_RETURN;
			  yyval.command = yyvsp[0].command;
			}
break;
case 130:
#line 804 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = command_connect (yyvsp[-3].command, yyvsp[0].command, '|'); }
break;
case 131:
#line 806 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.command = yyvsp[0].command; }
break;
case 132:
#line 810 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.number = CMD_TIME_PIPELINE; }
break;
case 133:
#line 812 "/usr/homes/chet/src/bash/src/parse.y"
{ yyval.number = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
break;
#line 5117 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
