#include <stdio.h>

#define NRW        11     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       10     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

enum symtype
{
<<<<<<< HEAD
<<<<<<< HEAD
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_AND,
	SYM_OR,
	SYM_NOT,
=======
=======
>>>>>>> parent of f50d058... 代码格式化
	SYM_NULL,			//空
	SYM_IDENTIFIER,		//标识符
	SYM_NUMBER,			//常数
	SYM_PLUS,			//符号	+
	SYM_MINUS,			//符号	-
	SYM_TIMES,			//符号	*
	SYM_SLASH,			//符号	/
	SYM_ODD,			//符号	odd
	SYM_EQU,			//符号	==
	SYM_NEQ,			//符号	!=
	SYM_LES,			//符号	<
	SYM_LEQ,			//符号	<=
	SYM_GTR,			//符号	>
	SYM_GEQ,			//符号	>=
	SYM_LPAREN,			//符号	(
	SYM_RPAREN,			//符号	)
	SYM_COMMA,			//符号	,
	SYM_SEMICOLON,		//符号	;
	SYM_PERIOD,			//符号	.
	SYM_BECOMES,		//符号	:=
	SYM_BEGIN,			//关键字	begin
	SYM_END,			//关键字	end
	SYM_IF,				//关键字	if
	SYM_THEN,			//关键字	then
	SYM_WHILE,			//关键字	while
	SYM_DO,				//关键字	do
	SYM_CALL,			//关键字	call
	SYM_CONST,			//关键字	const
	SYM_VAR,			//关键字	var
	SYM_PROCEDURE,		//关键字	procedure
	SYM_AND,			//符号	&&
	SYM_OR,				//符号	||
	SYM_NOT,			//符号	!
	SYM_LBRACK,			//符号	[
	SYM_RBRACK			//符号	]
<<<<<<< HEAD
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
};

enum idtype
{
<<<<<<< HEAD
<<<<<<< HEAD
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE
=======
=======
>>>>>>> parent of f50d058... 代码格式化
	ID_CONSTANT,
	ID_VARIABLE,
	ID_PROCEDURE,
	ID_ARRAY
<<<<<<< HEAD
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
<<<<<<< HEAD
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "",
/* 27 */    "",
/* 28 */    "",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;
=======
	/*  0 */ "",
	/*  1 */ "Found ':=' when expecting '='.",
	/*  2 */ "There must be a number to follow '='.",
	/*  3 */ "There must be an '=' to follow the identifier.",
	/*  4 */ "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */ "Missing ',' or ';'.",
	/*  6 */ "Incorrect procedure name.",
	/*  7 */ "Statement expected.",
	/*  8 */ "Follow the statement is an incorrect symbol.",
	/*  9 */ "'.' expected.",
	/* 10 */ "';' expected.",
	/* 11 */ "Undeclared identifier.",
	/* 12 */ "Illegal assignment.",
	/* 13 */ "':=' expected.",
	/* 14 */ "There must be an identifier to follow the 'call'.",
	/* 15 */ "A constant or variable can not be called.",
	/* 16 */ "'then' expected.",
	/* 17 */ "';' or 'end' expected.",
	/* 18 */ "'do' expected.",
	/* 19 */ "Incorrect symbol.",
	/* 20 */ "Relative operators expected.",
	/* 21 */ "Procedure identifier can not be in an expression.",
	/* 22 */ "Missing ')'.",
	/* 23 */ "The symbol can not be followed by a factor.",
	/* 24 */ "The symbol can not be as the beginning of an expression.",
	/* 25 */ "The number is too great.",
	/* 26 */ "Procedure identifier can not be in an array declaration",
	/* 27 */ "",
	/* 28 */ "",
	/* 29 */ "",
	/* 30 */ "",
	/* 31 */ "",
	/* 32 */ "There are too many levels." };

char ch;			   //上次读取的字符
int sym;			   //上次读取的符号
char id[MAXIDLEN + 1]; //上次读取的标识符
int num;			   //上次读取的数字
int cc;				   //字符数
int ll;				   //line length
int kk;
int err;
int cx;					//要生成的当前指令的索引
int level = 0;
int tx = 0;				//符号表索引
>>>>>>> parent of f50d058... 代码格式化

char line[80];

instruction code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';'
};

#define MAXINS   8
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
} mask;

FILE* infile;

// EOF PL0.h
