typedef union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
} YYSTYPE;
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


extern YYSTYPE yylval;
