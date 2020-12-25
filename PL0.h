#include <stdio.h>
#define NRW 15				//保留字数
#define TXMAX 500			//标识符表的长度
#define MAXNUMLEN 14		//数字的最大位数
#define NSYM 10				//数组ssym和csym中的最大符号数
#define MAXIDLEN 10			//标识符长度
#define MAXADDRESS 32767	//最大地址
#define MAXLEVEL 32			//嵌套块最大深度
#define CXMAX 500			//代码数组的大小
#define MAXSYM 30			//最大符号数
#define STACKSIZE 1000		//最大存储量
#define MAXDIM 10			//数组维度上限
#define MAXINS 12			//机器指令种类上限
#define MAXLABEL 20			//标签数目上限
#define MAXGOTOINS 20		//跳转指令数目上限

//符号类型
enum symtype
{
	SYM_NULL,			//NULL
	SYM_IDENTIFIER,		//标识符
	SYM_NUMBER,			//常数
	SYM_PLUS,			//符号	+
	SYM_MINUS,			//符号	-
	SYM_TIMES,			//符号	*
	SYM_SLASH,			//符号	/
	SYM_ODD,			//符号	odd（判定奇偶性）
	SYM_EQU,			//符号	=
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
	SYM_COLON,			//符号	:
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
	SYM_QUOTE,			//符号	&
	SYM_GOTO,			//关键字	goto
	SYM_ELSE,			//关键字	else
	SYM_RDM,			//关键字	random
	SYM_PRT				//关键字	prtint
};

//标识符的类型
enum idtype
{
	ID_CONSTANT,	//常数
	ID_VARIABLE,	//值类型变量
	ID_PROCEDURE,	//过程
	ID_ARRAY,		//数组
	ID_REFERENCE	//引用类型
};

//PL0处理机指令集
enum opcode
{
	LIT,	//将常数置于栈顶
	OPR,	//一组算术或逻辑运算指令
	LOD,	//将变量值置于栈顶
	STO,	//将栈顶的值赋与某变量
	CAL,	//用于过程调用的指令
	INT,	//在数据栈中分配存贮空间
	JMP,	//用于if, while 语句的条件或无条件控制转移指令
	JPC,	//栈顶值为0则跳转
	LDA,	//将栈顶指向的内存的值置于栈顶
	STA,	//将栈顶的值赋予栈顶-1指向的内存
	RDM,	//产生随机数
	PRT		//打印
};

//PL0处理机运算集
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
	int f;	//function code
	int l;	//level
	int a;	//displacement address
}instruction;

//错误信息库
char* errorMessage[] =
{
	/*  0 */ "",
	/*  1 */ "Found ':=' when expecting '='.",
	/*  2 */ "There must be a number to follow '='.",
	/*  3 */ "There must be an '=' to follow the identifier.",
	/*  4 */ "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */ "Missing ',' or ';'.",
	/*  6 */ "Incorrect procedure name.",
	/*  7 */ "Statement expected.",
	/*  8 */ "Follow the Statement is an incorrect symbol.",
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
	/* 21 */ "Procedure identifier can not be in an Expression.",
	/* 22 */ "Missing ')'.",
	/* 23 */ "The symbol can not be followed by a Factor.",
	/* 24 */ "The symbol can not be as the beginning of an Expression.",
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
	/* 38 */ "goto undefined label." };

char lastCharacter;				//上次读取的字符
int lastSymbol;					//上次读取的符号
char lastIdName[MAXIDLEN + 1];	//上次读取的标识符
int dimDateArray;					//上次读取的数字
int characterCount;				//字符数
int lineLenth;					//行长度
int kk;
int errorCount;					//错误数量
int currentInstructionIndex;	//要生成的当前指令的索引
int level = 0;			//当前层
int tableIndex = 0;				//符号表索引

char line[80];

//代码集合
instruction code[CXMAX];

//关键字集合
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end", "if",
	"odd", "procedure", "then", "var", "while", "goto", "else",
	"random", "print" };

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE, SYM_GOTO, SYM_ELSE,
	SYM_RDM, SYM_PRT
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_LPAREN,
	SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '(', ')', '=', ',', '.', ';'
};

//PL0指令集字符串集合
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "LDA", "STA", "RDM", "PRT" 
};

//数组附加属性
typedef struct
{
	int dim;					//维度
	short address;				//首地址
	short level;			//层级
	int dimDateArray[MAXDIM];	//每一维的向量数
	int dimSizeArray[MAXDIM];	//每一维的容量
	int totalSize;				//数组总容量
}attribute;

//常数符号
typedef struct
{
	char name[MAXIDLEN + 1];	//标识符的名称
	int kind;					//标识符的类型
	int value;					//标识符的值
}comtab;

/*
符号表
通过enter添加条目
有const、variable、procedure、array、quote五种类型
目前暂时将array单独处理在arrayTable里
onst使用comtab
quote使用qmask
其余使用mask
*/
comtab table[TXMAX];

//变量符号
typedef struct
{
	char name[MAXIDLEN + 1];	//标识符的名称
	int kind;					//标识符的类型
	short level;				//标识符的层级
	short address;				//标识符对应内容的地址
}mask;

//数组名符号
typedef struct
{
	char name[MAXIDLEN + 1];
	int kind;
	attribute* attribute;
}arrayMask;

arrayMask lastArray;		//上次读取的数组名
arrayMask currentArray;		//当前分析的数组名
int currentArrayDim;		//当前分析的数组维度大小

/*
由于分析goto时，label不一定已经分析过，因此无法立即生成指令
采取回填策略，在分析结束、执行之前，对goto产生的jmp指令的目标进行统一回填
*/

char gotoInstNameTab[MAXGOTOINS + 1][MAXIDLEN];		//goto指令名称集合
int gotoInstIndexTab[MAXGOTOINS + 1];				//goto指令索引集合
int gotoInstCount;									//goto指令数目
char labelNameTab[MAXLABEL + 1][MAXIDLEN];			//标签名称集合
int labelIndexTab[MAXLABEL + 1];					//标签索引集合
int labelCount;										//标签数目

//编译文件指针
FILE* infile;