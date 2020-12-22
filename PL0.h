#include <stdio.h>

#define NRW 11				//保留字数
#define TXMAX 500			//标识符表的长度
#define MAXNUMLEN 14		//数字的最大位数
#define NSYM 9				//数组ssym和csym中的最大符号数
#define MAXIDLEN 10			//标识符长度
#define MAXADDRESS 32767	//最大地址
#define MAXLEVEL 32			//嵌套块最大深度
#define CXMAX 500			//代码数组的大小
#define MAXSYM 30			//最大符号数
#define STACKSIZE 1000		//最大存储量

enum symtype
{
	SYM_NULL,			//NULL
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
	SYM_RBRACK,			//符号	]
	SYM_QUOTE			//符号	&
};

//标识符的类型
enum idtype
{
	ID_CONSTANT,	//常数
	ID_VARIABLE,	//变量
	ID_PROCEDURE,	//过程
	ID_ARRAY,		//数组
	ID_REFERENCE	//引用
};

enum opcode
{
	LIT,	//将常数置于栈顶
	OPR,	//一组算术或逻辑运算指令
	LOD,	//将变量值置于栈顶
	STO,	//将栈顶的值赋与某变量
	CAL,	//用于过程调用的指令
	INT,	//在数据栈中分配存贮空间
	JMP,	//用于if, while 语句的条件或无条件控制转移指令
	JPC		//一组算术或逻辑运算指令
};

enum oprcode
{
	OPR_RET,	//return
	OPR_NEG,	//算术运算符	取负
	OPR_ADD,	//算术运算符	加
	OPR_MIN,	//算术运算符	减
	OPR_MUL,	//算术运算符	乘
	OPR_DIV,	//算术运算符	除
	OPR_ODD,	//odd
	OPR_EQU,	//逻辑运算符	等于
	OPR_NEQ,	//逻辑运算符	不等于
	OPR_LES,	//逻辑运算符	小于
	OPR_LEQ,	//逻辑运算符	小于等于
	OPR_GTR,	//逻辑运算符	大于
	OPR_GEQ,	//逻辑运算符	大于等于
	OPR_AND,	//逻辑运算符	与
	OPR_OR,		//逻辑运算符	或
	OPR_NOT,	//逻辑运算符	非
};

//三格式指令FLA
typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//错误信息库
char* err_msg[] =
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
	/* 27 */ "There must be an identifier to follow '&'.",
	/* 28 */ "",
	/* 29 */ "",
	/* 30 */ "",
	/* 31 */ "",
	/* 32 */ "There are too many levels." };

char ch;				//上次读取的字符
int sym;				//上次读取的符号
char id[MAXIDLEN + 1];	//上次读取的标识符
int num;				//上次读取的数字
int cc;					//字符数
int ll;					//line length
int kk;
int err;
int cx;					//要生成的当前指令的索引
int level = 0;
int tx = 0;				//符号表索引

char line[80];

instruction code[CXMAX];

//关键字集合
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end", "if",
	"odd", "procedure", "then", "var", "while" };

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE };

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON };

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '(', ')', '=', ',', '.', ';' };


#define MAXINS 8	//PL0处理机指令集大小

//PL0处理机指令字符串集
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC"
};

typedef struct //comtab与mask占用同样的内存空间
{
	char name[MAXIDLEN + 1];
	int kind;
	int value;
}comtab;

comtab table[TXMAX]; //符号表，通过enter添加条目，有const、variable、procedure、array四种类型，const使用comtab存储，其他三种使用mask

typedef struct //mask与comtab占用同样的内存空间
{
	char name[MAXIDLEN + 1];
	int kind;
	short level;
	short address;
} mask;

FILE* infile;