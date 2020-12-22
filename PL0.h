#include <stdio.h>

#define NRW 11		 // number of reserved words
#define TXMAX 500	 // length of identifier table
#define MAXNUMLEN 14 // maximum number of digits in numbers
#define NSYM 9		 // maximum number of symbols in array ssym and csym
#define MAXIDLEN 10	 // length of identifiers

#define MAXDIM 10 //maximum dimension of array

#define MAXADDRESS 32767 // maximum address
#define MAXLEVEL 32		 // maximum depth of nesting block
#define CXMAX 500		 // size of code array

#define MAXSYM 30 // maximum number of symbols

#define STACKSIZE 1000 // maximum storage

enum symtype
{
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
	SYM_AND,	// &&
	SYM_OR,		// ||
	SYM_NOT,	// !
	SYM_LBRACK, // [
	SYM_RBRACK, // ]
	SYM_ARRAY	// array
};

enum idtype
{
	ID_CONSTANT,
	ID_VARIABLE,
	ID_PROCEDURE,
	ID_ARRAY
};

enum opcode
{
	LIT,
	OPR,
	LOD,
	STO,
	CAL,
	INT,
	JMP,
	JPC
};

enum oprcode
{
	OPR_RET,
	OPR_NEG,
	OPR_ADD,
	OPR_MIN,
	OPR_MUL,
	OPR_DIV,
	OPR_ODD,
	OPR_EQU,
	OPR_NEQ,
	OPR_LES,
	OPR_LEQ,
	OPR_GTR,
	OPR_GEQ,
	OPR_AND, //&&
	OPR_OR,	 //||
	OPR_NOT, //!
};

typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char *err_msg[] =
	{
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
		/* 27 */ "expected ']'",
		/* 28 */ "",
		/* 29 */ "",
		/* 30 */ "",
		/* 31 */ "",
		/* 32 */ "There are too many levels."};

//////////////////////////////////////////////////////////////////////
char ch;			   // last character read
int sym;			   // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int num;			   // last number read
int cc;				   // character count
int ll;				   // line length
int kk;
int err;
int cx; // index of current instruction to be generated.
int level = 0;
int tx = 0; //index of table
int ax = 0; //index of array_table

char line[80];

instruction code[CXMAX];

char *word[NRW + 1] =
	{
		"", /* place holder */
		"begin", "call", "const", "do", "end", "if",
		"odd", "procedure", "then", "var", "while"};

int wsym[NRW + 1] =
	{
		SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
		SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE};

int ssym[NSYM + 1] =
	{
		SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES,
		SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON};

char csym[NSYM + 1] =
	{
		' ', '+', '-', '*', '(', ')', '=', ',', '.', ';'};

#define MAXINS 8
char *mnemonic[MAXINS] =
	{
		"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC"};

typedef struct //数组附加属性
{
	int dim;		  //维度 a[3][4]的维度记为2
	short address;	  //在符号表中的首地址
	short level;	  //层次
	int num[MAXDIM];  //记录声明时的数据，如a[3][4],num[0] = 3,num[1] = 4
	int size[MAXDIM]; //记录对应维度规模的大小,如a[3][4],size[0] = 4,size[1] = 1;
	int sum;		  //该数组总元素个数，max_index = num[0] * size[0];
} attribute;
// a[2][1]  address + 2 * 4 + 1 * 1 = address + 9
/*
1  2  3  4
5  6  7  8
9  10 11 12
*/
typedef struct //const使用
{
	char name[MAXIDLEN + 1];
	int kind;
	int value;
} comtab;

comtab table[TXMAX]; //符号表，通过enter添加条目，有const、variable、procedure三种种类型

typedef struct //variable与procedure使用
{
	char name[MAXIDLEN + 1];
	int kind;
	short level;
	short address;//栈中地址
} mask;

typedef struct mask_array //array使用
{
	char name[MAXIDLEN + 1];
	int kind;
	attribute *attr;
} mask_array;

mask_array mk_a;
mask_array array_table[TXMAX]; //专门存放数组的符号表
int cur_dim;
int array_link[MAXDIM];

FILE *infile;

// EOF PL0.h
