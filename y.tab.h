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


extern YYSTYPE yylval;
