#include <stdio.h>

#define NRW 15		 // number of reserved words
#define TXMAX 500	 // length of identifier table
#define MAXNUMLEN 14 // maximum number of digits in numbers
#define NSYM 10		 // maximum number of symbols in array ssym and csym
#define MAXIDLEN 10	 // length of identifiers

#define MAXDIM 10 //maximum dimension of array

#define MAXADDRESS 32767 // maximum address
#define MAXLEVEL 32		 // maximum depth of nesting block
#define CXMAX 500		 // size of code array

#define MAXSYM 41 // maximum number of symbols

#define STACKSIZE 1000 // maximum storage

#define MAXPROCEDURE 50 // 函数最大数量

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
	SYM_GOTO,	// goto
	SYM_ELSE,	// else
	SYM_RDM,	// random
	SYM_PRT,	// print
	SYM_COLON,	// :
<<<<<<< HEAD
<<<<<<< HEAD
	SYM_QUOTE,	// &
=======
	SYM_QUOTE,	//&
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
=======
	SYM_QUOTE,	//&
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
};

enum idtype
{
	ID_CONSTANT,
	ID_VARIABLE,
	ID_PROCEDURE,
	ID_ARRAY,
	ID_REFERENCE,
<<<<<<< HEAD
<<<<<<< HEAD
	ID_PARAMETER_I, //引用参数
	ID_PARAMETER_A	//数组参数
=======
	ID_PARAMETER_I,//引用参数
	ID_PARAMETER_A//数组参数
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
=======
	ID_PARAMETER_I,//引用参数
	ID_PARAMETER_A//数组参数
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
};

enum opcode
{ //增加LDA和STA用于数组，新增RDM和PRT用于打印
	LIT,
	OPR,
	LOD,
	STO,
	CAL,
	INT,
	JMP,
	JPC, //栈顶值为0则跳转
	LDA,
	STA,
	RDM, //random
	PRT,
	PAS, //用于传参
	LDP, //以下为参数数组读取所需指令（传地址）
	STP, //针对存参数数组
	STI	 //针对存参数引用
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
		/* 26 */ "Procedure identifier can not be in an array declaration.",
		/* 27 */ "expected ']'.",
		/* 28 */ "expected a constant or a number.",
		/* 29 */ "There must be an identify to follow '&'.",
		/* 30 */ "The reference does not initial.",
		/* 31 */ "There must be a identify to follow '='.",
		/* 32 */ "There are too many levels.",
		/* 33 */ "Missing '('.",
		/* 34 */ "the same label has been used.",
		/* 35 */ "too many labels.",
		/* 36 */ "Missing ';'.",
		/* 37 */ "too many goto.",
		/* 38 */ "goto undefined label.",
		/* 39 */ "missing '['.",
		/* 40 */ "incompatible parameters.",
		/* 41 */ "incorrect parameter passing, wrong number or missing ')'.",
		/* 42 */ "expecting '&' or identifier of ID_VARIABLE"};

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

char line[80];

instruction code[CXMAX];

char *word[NRW + 1] =
	{
		"", /* place holder */
		"begin", "call", "const", "do", "end", "if",
		"odd", "procedure", "then", "var", "while", "goto", "else",
		"random", "print"};

int wsym[NRW + 1] =
	{
		SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
		SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE, SYM_GOTO, SYM_ELSE,
		SYM_RDM, SYM_PRT};

int ssym[NSYM + 1] =
	{
		SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_LPAREN,
		SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON};

char csym[NSYM + 1] =
	{
		' ', '+', '-', '*', '(', ')', '=', ',', '.', ';'};

#define MAXINS 16 //增加LDA和STA用于数组，
char *mnemonic[MAXINS] =
	{
<<<<<<< HEAD
<<<<<<< HEAD
		"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "LDA", "STA", "RDM", "PRT",
=======
		"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "LDA", "STA", "RDM", "PRT", 
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
=======
		"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "LDA", "STA", "RDM", "PRT", 
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
		"PAS", "LDP", "STP", "STI"};

typedef struct //数组附加属性
{
	int dim;		  //维度 a[3][4]的维度记为2
	short address;	  //在栈中的首地址
	short level;	  //层次
	int num[MAXDIM];  //记录声明时的数据，如a[3][4],num[0] = 3,num[1] = 4
	int size[MAXDIM]; //记录对应维度规模的大小,如a[3][4],size[0] = 4,size[1] = 1;
	int sum;		  //该数组总元素个数，max_index = num[0] * size[0];
} attribute;
// a[2][1]  address + 2 * 4 + 1 * 1 = address + 9
<<<<<<< HEAD
<<<<<<< HEAD
=======
=======
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
/*
1  2  3  4
5  6  7  8
9  10 11 12
*/
<<<<<<< HEAD
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
=======
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191

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
<<<<<<< HEAD
<<<<<<< HEAD
	short level;   //procedure中保存函数表中偏移
=======
	short level;	//procedure中保存函数表中偏移
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
=======
	short level;	//procedure中保存函数表中偏移
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
	short address; //栈中地址
} mask;

typedef struct mask_array //array使用
{
	char name[MAXIDLEN + 1];
	int kind;
	attribute *attr;
} mask_array;

/////建立函数参数表
typedef struct procedure_parameter
{
<<<<<<< HEAD
<<<<<<< HEAD
	int kind; //参数类型  传值，传地址，传数组
	struct procedure_parameter *next;
} procedure_parameter;

typedef struct
{
	int para_num; //参数数量
	procedure_parameter *next;
} procedure_head;
=======
=======
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
	int kind;	//参数类型  传值，传地址，传数组
	struct procedure_parameter *next;
}procedure_parameter;

typedef struct 
{
	int para_num;	//参数数量
	procedure_parameter *next; 
}procedure_head;
<<<<<<< HEAD
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191
=======
>>>>>>> d58c4d173ec09e8fa5c776b724a6d7ebb424c191

procedure_head all_procedure[MAXPROCEDURE]; //全体函数参量表
short now_procedure;

mask_array lastArray; //最后读到的数组声明
mask_array curArray;  //当前正在分析的数组
int cur_dim;

FILE *infile;

int parameter_num;

#define NLABEL 20					   //label的最大数量
#define NGOTO 20					   //goto的最大数量
char goto_dest[NGOTO + 1][MAXIDLEN];   //记录每个goto的目标
int goto_cx[NGOTO + 1];				   //记录每个goto所在位置是第几条指令
int goto_num;						   //记录goto的总数量
char label_name[NLABEL + 1][MAXIDLEN]; //记录每个label的名字
int label_cx[NLABEL + 1];			   //记录每个label所在位置是第几条指令
int label_num;						   //记录label的总数量
//由于分析goto时，label不一定已经分析过，因此无法立即生成指令
//采取回填策略，在分析结束、执行之前，对goto产生的jmp指令的目标进行统一回填

// EOF PL0.h
