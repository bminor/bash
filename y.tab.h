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
#define	COND_START	272
#define	COND_END	273
#define	COND_ERROR	274
#define	IN	275
#define	BANG	276
#define	TIME	277
#define	TIMEOPT	278
#define	WORD	279
#define	ASSIGNMENT_WORD	280
#define	NUMBER	281
#define	ARITH_CMD	282
#define	ARITH_FOR_EXPRS	283
#define	COND_CMD	284
#define	AND_AND	285
#define	OR_OR	286
#define	GREATER_GREATER	287
#define	LESS_LESS	288
#define	LESS_AND	289
#define	GREATER_AND	290
#define	SEMI_SEMI	291
#define	LESS_LESS_MINUS	292
#define	AND_GREATER	293
#define	LESS_GREATER	294
#define	GREATER_BAR	295
#define	yacc_EOF	296


extern YYSTYPE yylval;
