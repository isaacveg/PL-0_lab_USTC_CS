#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "PL0.h"
#include "set.c"

void Expression(symset fsys);

//打印错误信息
void Error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= characterCount - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, errorMessage[n]);
	errorCount++;
}

/*
2.1 词法分析
PL/0 的语言的词法分析器将要完成以下工作：
（1） 跳过分隔符（如空格，回车，制表符）；
（2） 识别诸如begin，end，if，while 等保留字；
（3） 识别非保留字的一般标识符，此标识符值（字符序列）赋给全局量id，
而全局量sym 赋值为SYM_IDENTIFIER。
（4） 识别数字序列，当前值赋给全局量NUM，sym 则置为SYM_NUMBER；
（5） 识别:=，<=，>=之类的特殊符号，全局量sym 则分别被赋值为
SYM_BECOMES，SYM_LEQ，SYM_GTR 等。
*/

/*
获取单个字符的过程，除此之外，它还完成：
（1） 识别且跳过行结束符；
（2） 将输入源文件复写到输出文件；
（3） 产生一份程序列表，输出相应行号或指令计数器的值。
*/
void GetCharacter(void)
{
	if (characterCount == lineLenth)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		lineLenth = characterCount = 0;
		printf("%5d  ", currentInstructionIndex);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((lastCharacter = getc(infile)) != '\n'))
		{
			printf("%c", lastCharacter);
			line[++lineLenth] = lastCharacter;
		} // while
		printf("\n");
		line[++lineLenth] = ' ';
	}
	lastCharacter = line[++characterCount];
}

//获取输入符号
void GetSymbol(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (lastCharacter == ' ' || lastCharacter == '\t')
		GetCharacter();

	if (isalpha(lastCharacter))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = lastCharacter;
			GetCharacter();
		} while (isalpha(lastCharacter) || isdigit(lastCharacter));
		a[k] = 0;
		strcpy(lastIdName, a);
		word[0] = lastIdName;
		i = NRW;
		while (strcmp(lastIdName, word[i--]))
			;
		if (++i)
			lastSymbol = wsym[i]; // symbol is a reserved word
		else
			lastSymbol = SYM_IDENTIFIER; // symbol is an identifier
	}
	else if (isdigit(lastCharacter))
	{ // symbol is a number.
		k = dimDateArray = 0;
		lastSymbol = SYM_NUMBER;
		do
		{
			dimDateArray = dimDateArray * 10 + lastCharacter - '0';
			k++;
			GetCharacter();
		} while (isdigit(lastCharacter));
		if (k > MAXNUMLEN)
			Error(25); // The number is too great.
	}
	else if (lastCharacter == ':')
	{
		GetCharacter();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_BECOMES; // :=
			GetCharacter();
		}
		else
		{
			lastSymbol = SYM_COLON; // :
		}
	}
	else if (lastCharacter == '>')
	{
		GetCharacter();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_GEQ; // >=
			GetCharacter();
		}
		else
		{
			lastSymbol = SYM_GTR; // >
		}
	}
	else if (lastCharacter == '<')
	{
		GetCharacter();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_LEQ; // <=
			GetCharacter();
		}
		else if (lastCharacter == '>')
		{
			lastSymbol = SYM_NEQ; // <>
			GetCharacter();
		}
		else
		{
			lastSymbol = SYM_LES; // <
		}
	}
	else if (lastCharacter == '&')
	{
		GetCharacter();
		if (lastCharacter == '&')
		{
			lastSymbol = SYM_AND; // &&
			GetCharacter();
		}
		else
			lastSymbol = SYM_QUOTE;
	}
	else if (lastCharacter == '|')
	{
		GetCharacter();
		if (lastCharacter == '|')
		{
			lastSymbol = SYM_OR; // ||
			GetCharacter();
		}
	}
	else if (lastCharacter == '!')
	{
		lastSymbol = SYM_NOT; //!
		GetCharacter();
	}
	else if (lastCharacter == '[')
	{
		lastSymbol = SYM_LBRACK;
		GetCharacter();
	}
	else if (lastCharacter == ']')
	{
		lastSymbol = SYM_RBRACK;
		GetCharacter();
	}
	else if (lastCharacter == '=')
	{
		GetCharacter();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_EQU;
			GetCharacter();
		}
		else
			lastSymbol = SYM_ASSIGN;
	}
	else if (lastCharacter == '/')
		//为实现注释，将对'/'的匹配从else中删除（即删除csym与ssym中的slash）,挪到此处
	{
		GetCharacter();
		if (lastCharacter == '/') // 读到"//"
		{
			int tag = 1;
			while (tag)
			{
				GetCharacter();
				if (characterCount == lineLenth) //读完本行
				{
					tag = 0;
					GetCharacter();
				}
			}
			GetSymbol();
		}
		else if (lastCharacter == '*') // 读到"/*"
		{
			int tag = 1;
			while (tag)
			{
				GetCharacter();
				if (lastCharacter == '*')
				{
					GetCharacter();
					if (lastCharacter == '/') //读到"*/"
					{
						tag = 0;
						GetCharacter();
					}
				}
			}
			GetSymbol();
		}
		else
		{
			lastSymbol = SYM_SLASH;
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = lastCharacter;
		while (csym[i--] != lastCharacter)
			;
		if (++i)
		{
			lastSymbol = ssym[i];
			GetCharacter();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
}

/*
2.4 代码生成
PL/0 编译程序不仅完成通常的词法分析、语法分析，而且还产生中间代码和
“目标”代码。最终我们要“运行”该目标码。为了使我们的编译程序保持适当
简单的水平，不致陷入与本课程无关的实际机器的特有性质的考虑中去，我们假
想有台适合PL/0 程序运行的计算机，我们称之为PL/0 处理机。PL/0 处理机顺
序解释生成的目标代码，我们称之为解释程序。注意：这里的假设与我们的编译
概念并不矛盾，在本课程中我们写的只是一个示范性的编译程序，它的后端无法
完整地实现，因而只能在一个解释性的环境下予以模拟。从另一个角度上讲，把
解释程序就看成是PL/0 机硬件，把解释执行看成是PL/0 的硬件执行，那么我们
所做的工作：由PL/0 源语言程序到PL/0 机器指令的变换，就是一个完整的编译
程序。
PL/0 处理机有两类存贮，目标代码放在一个固定的存贮数组code 中，而所
需数据组织成一个栈形式存放。
PL/0 处理机的指令集根据PL/0 语言的要求而设计，它包括以下的指令：
（1）LIT 将常数置于栈顶
（2）LOD 将变量值置于栈顶
（3）STO 将栈顶的值赋与某变量
（4）CAL 用于过程调用的指令
（5）INT 在数据栈中分配存贮空间
（6）JMP, JPC  用于if, while 语句的条件或无条件控制转移指令
（7）OPR 一组算术或逻辑运算指令
*/

/*
生成中间代码
把三个参数f、l、a 组装成一条目标指令并存放于code 数组中，增加CX 的值，CX 表示下一条即将生成的目标指令的地址。
*/
void Generate(int x, int y, int z)
{
	if (currentInstructionIndex > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[currentInstructionIndex].f = x;
	code[currentInstructionIndex].l = y;
	code[currentInstructionIndex++].a = z;
}

/*
2.7 符号表管理
为了组成一条指令，编译程序必须知道其操作码及其参数（数或地址）。这
些值是由编译程序本身联系到相应标识符上去的。这种联系是在处理常数、变量
和过程说明完成的。为此，标识符表应包含每一标识符所联系的属性；如果标识
符被说明为常数，其属性值为常数值；如果标识符被说明成变量，其属性就是由
层次和修正量（偏移量）组成的地址；如果标识符被说明为过程，其属性就是过
程的入口地址及层次。
常数的值由程序正文提供，编译的任务就是确定存放该值的地址。我们选择
16
顺序分配变量和代码的方法；每遇到一个变量说明，就将数据单元的下标加一
（PL/0 机中，每个变量占一个存贮单元）。开始编译一个过程时，要对数据单元
的下标dateIndex 赋初值，表示新开辟一个数据区。dateIndex 的初值为3，因为每个数据区包
含三个内部变量RA，DL 和SL。
*/

//测试是否发生错误并跳过不属于s1或s2的所有符号
void Test(symset s1, symset s2, int n)
{
	symset s;

	if (!inset(lastSymbol, s1))
	{
		Error(n);
		s = uniteset(s1, s2);
		while (!inset(lastSymbol, s))
			GetSymbol();
		destroyset(s);
	}
}

int dateIndex;	 //数据分配索引

/*
向符号表添加新的符号，并确定标识符的有关属性
常量定义：通过循环，反复获得标识符和对应的值，存入符号表。符号表中记录下标识符的名字和它对应的值。
变量定义：通过循环，反复获得标识符，存入符号表。符号表中记录下标识符的名字、它所在的层及它在所在层中的偏移地址。
*/
void Enter(int kind)
{
	mask* mk;
	arrayMask* mk_a;

	tableIndex++;
	strcpy(table[tableIndex].name, lastIdName);
	table[tableIndex].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (dimDateArray > MAXADDRESS)
		{
			Error(25); // The number is too great.
			dimDateArray = 0;
		}
		table[tableIndex].value = dimDateArray;
		break;
	case ID_VARIABLE:
		mk = (mask*)& table[tableIndex];
		mk->level = level;
		mk->address = dateIndex++;
		break;
	case ID_PROCEDURE:
		mk = (mask*)& table[tableIndex];
		mk->level = level;
		break;
	case ID_ARRAY:
		lastArray.attribute->dim = currentArrayDim;
		lastArray.attribute->dimSizeArray[currentArrayDim - 1] = 1;
		lastArray.attribute->level = level;
		for (int i = currentArrayDim - 1; i > 0; i--)
		{
			lastArray.attribute->dimSizeArray[i - 1] = lastArray.attribute->dimSizeArray[i] * lastArray.attribute->dimDateArray[i]; //计算每个维度的dimSizeArray
		}
		lastArray.attribute->totalSize = lastArray.attribute->dimSizeArray[0] * lastArray.attribute->dimDateArray[0]; //计算totalSize
		mk_a = (arrayMask*)& table[tableIndex];
		*mk_a = lastArray;										 //至此完成name，dim，level，dimDateArray，dimSizeArray，totalSize的修改，还差address
		mk_a->attribute->address = dateIndex;								 //dateIndex作为首地址
		dateIndex += mk_a->attribute->totalSize;									 //为数组开辟totalSize大小的空间
		lastArray.attribute = (attribute*)malloc(sizeof(attribute)); //先前为lastArray.attribute开辟的空间已经被table使用，开辟新的空间
		break;
	case ID_REFERENCE:
		mk = (mask*)& table[tableIndex];
		mk->level = level;
		break;
	} // switch
}

//在符号表中查找标识符并返回索引
int Position(char* lastIdName)
{
	int i;
	strcpy(table[0].name, lastIdName);
	i = tableIndex + 1;
	while (strcmp(table[--i].name, lastIdName) != 0)
		;
	return i;
}

//常数声明
void ConstDeclaration()
{
	if (lastSymbol == SYM_IDENTIFIER)
	{
		GetSymbol();
		if (lastSymbol == SYM_EQU || lastSymbol == SYM_BECOMES)
		{
			if (lastSymbol == SYM_BECOMES)
				Error(1); // Found ':=' when expecting '='.
			GetSymbol();
			if (lastSymbol == SYM_NUMBER)
			{
				Enter(ID_CONSTANT);
				GetSymbol();
			}
			else
			{
				Error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			Error(3); // There must be an '=' to follow the identifier.
		}
	}
	else
		Error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
}

//维度声明
int DimConst()
{
	if (lastSymbol == SYM_IDENTIFIER || lastSymbol == SYM_NUMBER)
	{ //identifier必是const类型
		int i;
		if (lastSymbol == SYM_IDENTIFIER)
		{
			if (!(i = Position(lastIdName)))
			{
				Error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				Error(26); // Illegal identifier.
				i = 0;
			}

			return table[i].value;
		}
		else
		{
			return dimDateArray;
		}
	}
	else
	{
		Error(28);
	}
}

//数组声明
void DimDeclaration(void)
{
	int value;
	if (lastSymbol == SYM_LBRACK)
	{
		GetSymbol();
		value = DimConst();

		lastArray.attribute->dimDateArray[currentArrayDim++] = value;

		GetSymbol();
		if (lastSymbol != SYM_RBRACK)
		{
			Error(27); //expected ']'
		}
		else
		{
			GetSymbol();
			DimDeclaration();
		}
	}
}

//变量声明
void Vardeclaration(void)
{
	if (lastSymbol == SYM_IDENTIFIER)
	{
		GetSymbol();
		if (lastSymbol == SYM_LBRACK)
		{ //标识符是数组
			currentArrayDim = 0;
			lastArray.kind = ID_ARRAY;
			strcpy(lastArray.name, lastIdName);
			DimDeclaration();
			Enter(ID_ARRAY);
		}
		else
			Enter(ID_VARIABLE);
	}
	else if (lastSymbol == SYM_QUOTE)
	{
		GetSymbol();
		if (lastSymbol == SYM_IDENTIFIER)
		{
			Enter(ID_REFERENCE);
			GetSymbol();
			if (lastSymbol == SYM_ASSIGN)
			{
				GetSymbol();
				if (lastSymbol == SYM_IDENTIFIER)
				{
					int i = Position(lastIdName);
					if (i != 0)
					{
						//对引用类型所指向的地址进行回填
						((mask*)table + tableIndex)->address = ((mask*)table + i)->address;
						GetSymbol();
					}
					else
						Error(11);	//Undeclared identifier.
				}
				else
					Error(31);	//There must be a identify to follow '='.
			}
			else
				Error(30);	//The reference does not initial.
		}
		else
			Error(29);	//There must be an identify to follow '&'.
	}
	else
		Error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
}

/*
每一个分程序（过程）被编译结束后，将列出该部分PL/0 程序代码。
注意，每个分程序（过程）的第一条指令未被列出。
该指令是跳转指令。
其作用是绕过该分程序的说明部分所产生的代码（含过程说明所产生的代码
*/
void ListCode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
}

//匹配数组的维度信息，并将偏移量置于栈顶
void MatchArrayDim(symset fsys)
{ //匹配数组的维度信息，并将偏移量置于栈顶
	symset set;
	currentArrayDim = 0;
	Generate(LIT, 0, 0);
	while (lastSymbol == SYM_LBRACK)
	{
		currentArrayDim++;
		GetSymbol();
		set = uniteset(createset(SYM_RBRACK, SYM_NULL), fsys);
		Expression(set);
		destroyset(set);
		GetSymbol();
		Generate(LIT, 0, currentArray.attribute->dimSizeArray[currentArrayDim - 1]);
		Generate(OPR, 0, OPR_MUL);
		Generate(OPR, 0, OPR_ADD);
	}
}

//因子
void Factor(symset fsys)
{
	void Expression(symset fsys);
	int i;
	symset set;

	Test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an Expression.

	if (inset(lastSymbol, facbegsys))
	{
		if (lastSymbol == SYM_IDENTIFIER || lastSymbol == ID_REFERENCE)
		{
			GetSymbol();
			if (lastSymbol == SYM_LBRACK)
			{ // array
				if (!(i = Position(lastIdName)))
				{
					Error(11); // Undeclared identifier.
				}
				else
				{
					arrayMask* mk = (arrayMask*)& table[i];
					currentArray = *mk;
					MatchArrayDim(fsys);

					Generate(LDA, level - mk->attribute->level, mk->attribute->address);
				}
			}
			else
			{ // variable
				if ((i = Position(lastIdName)) == 0)
				{
					Error(11); // Undeclared identifier.
				}
				else
				{
					switch (table[i].kind)
					{
						mask* mk;
					case ID_CONSTANT:
						Generate(LIT, 0, table[i].value);
						break;
					case ID_REFERENCE:
					case ID_VARIABLE:
						mk = (mask*)& table[i];
						Generate(LOD, level - mk->level, mk->address);
						break;
					case ID_PROCEDURE:
						Error(21); // Procedure identifier can not be in an Expression.
						break;
					} // switch
				}
			}
		}
		else if (lastSymbol == SYM_NUMBER)
		{
			if (dimDateArray > MAXADDRESS)
			{
				Error(25); // The number is too great.
				dimDateArray = 0;
			}
			Generate(LIT, 0, dimDateArray);
			GetSymbol();
		}
		else if (lastSymbol == SYM_LPAREN)
		{
			GetSymbol();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			Expression(set);
			destroyset(set);
			if (lastSymbol == SYM_RPAREN)
			{
				GetSymbol();
			}
			else
			{
				Error(22); // Missing ')'.
			}
		}
		else if (lastSymbol == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			GetSymbol();
			Factor(fsys);
			Generate(OPR, 0, OPR_NEG);
		}
		else if (lastSymbol == SYM_NOT)
		{
			GetSymbol();
			Factor(fsys);
			Generate(OPR, 0, OPR_NOT);
		}
		else if (lastSymbol == SYM_RDM)
		{
			GetSymbol();
			if (lastSymbol == SYM_LPAREN)
			{
				GetSymbol();
			}
			else
				Error(33);
			if (lastSymbol == SYM_RPAREN)
			{
				GetSymbol();
				Generate(RDM, 0, 0);
			}
			else if (lastSymbol == SYM_NUMBER)
			{
				GetSymbol();
				if (lastSymbol == SYM_RPAREN)
				{
					Generate(RDM, 0, dimDateArray);
					GetSymbol();
				}
				else
					Error(22);
			}
		}
		Test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
}

//项
void Term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	Factor(set);
	while (lastSymbol == SYM_TIMES || lastSymbol == SYM_SLASH)
	{
		mulop = lastSymbol;
		GetSymbol();
		Factor(set);
		if (mulop == SYM_TIMES)
		{
			Generate(OPR, 0, OPR_MUL);
		}
		else
		{
			Generate(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
}

//表达式
void Expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	Term(set);
	while (lastSymbol == SYM_PLUS || lastSymbol == SYM_MINUS)
	{
		addop = lastSymbol;
		GetSymbol();
		Term(set);
		if (addop == SYM_PLUS)
		{
			Generate(OPR, 0, OPR_ADD);
		}
		else
		{
			Generate(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
}
//一般优先级条件
void Condition(symset fsys)
{
	int relop;
	symset set;

	if (lastSymbol == SYM_ODD)
	{
		GetSymbol();
		Expression(fsys);
		Generate(OPR, 0, OPR_ODD);
	}
	else
	{
		set = uniteset(relset, fsys);
		Expression(set);
		destroyset(set);
		if (inset(lastSymbol, relset))
		{
			relop = lastSymbol;
			GetSymbol();
			set = uniteset(relset, fsys);
			Expression(set);
			destroyset(set);
			switch (relop)
			{
			case SYM_EQU:
				Generate(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				Generate(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				Generate(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				Generate(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				Generate(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				Generate(OPR, 0, OPR_LEQ);
				break;
			}
			// switch
		} // else
	}	  // else
}

//与优先级条件
void AndCondition(symset fsys)
{
	symset set, set1;

	set1 = createset(SYM_AND, SYM_NULL);
	set = uniteset(set1, fsys);
	Condition(set);
	while (lastSymbol == SYM_AND)
	{
		GetSymbol();
		Condition(set);
		Generate(OPR, 0, OPR_AND);
	}
	destroyset(set1);
	destroyset(set);
}

//或优先级条件
void OrCondition(symset fsys)
{
	symset set, set1;

	set1 = createset(SYM_OR, SYM_NULL);
	set = uniteset(set1, fsys);
	AndCondition(set);
	while (lastSymbol == SYM_OR)
	{
		GetSymbol();
		AndCondition(set);
		Generate(OPR, 0, OPR_OR);
	}
	destroyset(set1);
	destroyset(set);
}

//语句
void Statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (lastSymbol == SYM_IDENTIFIER)
	{
		int cur_lever, addr;
		GetSymbol();
		if (lastSymbol == SYM_LBRACK)
		{ // array assignment
			if (!(i = Position(lastIdName)))
			{
				Error(11); // Undeclared identifier.
			}

			arrayMask* mk = (arrayMask*)& table[i];
			currentArray = *mk;
			MatchArrayDim(fsys);

			if (lastSymbol == SYM_BECOMES)
			{
				GetSymbol();
			}
			else
			{
				Error(13); // ':=' expected.
			}

			set = uniteset(createset(SYM_RBRACK, SYM_NULL), fsys);
			Expression(fsys);
			destroyset(set);

			if (i)
				Generate(STA, level - mk->attribute->level, mk->attribute->address);
		}
		else
		{ // variable assignment
			mask* mk;
			if (!(i = Position(lastIdName)))
			{ //发现未定义的变量
				if (lastSymbol != SYM_COLON)
					Error(11); // Undeclared identifier.
				else		   //lastIdName是一个label
				{			   //开始对label的处理
					set1 = createset(SYM_COLON, SYM_NULL);
					fsys = uniteset(set1, fsys);
					destroyset(set1);

					strcpy(label_name[0], lastIdName);
					int k = label_num;
					while (strcmp(label_name[k--], lastIdName) != 0) //检查是否有重复的label
						;

					if (++k)
					{
						Error(34); //有重复的label
					}
					else
					{
						label_num++;
						if (label_num > MAXLABEL)
						{
							Error(35); //label过多
						}
						else
						{
							strcpy(label_name[label_num], lastIdName);
							label_cx[label_num] = currentInstructionIndex; //存放label对应的地址
						}
						GetSymbol();
						Test(fsys, phi, 19);
						Statement(fsys);
						return; //完成对 label: 的匹配
					}
				} //else 至此完成对label的处理
			}
			else if (table[i].kind != ID_VARIABLE && table[i].kind != ID_REFERENCE)
			{
				Error(12); // Illegal assignment.
				i = 0;
			}
			mk = (mask*)& table[i];

			if (lastSymbol == SYM_BECOMES)
			{
				GetSymbol();
			}
			else
			{
				Error(13); // ':=' expected.
			}

			Expression(fsys);

			if (i)
				Generate(STO, level - mk->level, mk->address);
		}
	}
	else if (lastSymbol == SYM_CALL)
	{ // procedure call
		GetSymbol();
		if (lastSymbol != SYM_IDENTIFIER)
		{
			Error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = Position(lastIdName)))
			{
				Error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*)& table[i];
				Generate(CAL, level - mk->level, mk->address);
			}
			else
			{
				Error(15); // A constant or variable can not be called.
			}
			GetSymbol();
		}
	}
	else if (lastSymbol == SYM_IF)
	{ // if Statement
		GetSymbol();

		if (lastSymbol == SYM_LPAREN)
		{
			GetSymbol();
		}
		else
			Error(33); //Missing '('.

		set1 = createset(SYM_RPAREN, SYM_NULL);
		set = uniteset(set1, fsys);
		OrCondition(set);
		destroyset(set1);
		destroyset(set);

		if (lastSymbol == SYM_RPAREN)
		{
			GetSymbol();
		}
		else
			Error(33); //Missing ')'.

		if (lastSymbol == SYM_THEN)
		{
			GetSymbol();
		}
		else
		{
			Error(16); // 'then' expected.
		}
		cx1 = currentInstructionIndex;
		Generate(JPC, 0, 0);
		Statement(fsys);
		code[cx1].a = currentInstructionIndex;
	}
	else if (lastSymbol == SYM_BEGIN)
	{ // Block
		GetSymbol();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		Statement(set);
		while (lastSymbol == SYM_SEMICOLON || inset(lastSymbol, statbegsys))
		{
			if (lastSymbol == SYM_SEMICOLON)
			{
				GetSymbol();
			}
			else
			{
				Error(10);
			}
			Statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (lastSymbol == SYM_END)
		{
			GetSymbol();
		}
		else
		{
			Error(17); // ';' or 'end' expected.
		}
	}
	else if (lastSymbol == SYM_WHILE)
	{ // while Statement
		cx1 = currentInstructionIndex;
		GetSymbol();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		Condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = currentInstructionIndex;
		Generate(JPC, 0, 0);
		if (lastSymbol == SYM_DO)
		{
			GetSymbol();
		}
		else
		{
			Error(18); // 'do' expected.
		}
		Statement(fsys);
		Generate(JMP, 0, cx1);
		code[cx2].a = currentInstructionIndex;
	}
	else if (lastSymbol == SYM_PRT)
	{ // while Statement
		GetSymbol();
		if (lastSymbol == SYM_LPAREN)
		{
			GetSymbol();
		}
		else
			Error(33);
		if (lastSymbol == SYM_RPAREN)
		{
			Generate(PRT, 0, 0);
			GetSymbol();
		}
		else
		{
			set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
			set1 = uniteset(fsys, set);
			Expression(set);
			destroyset(set1);
			destroyset(set);
			Generate(PRT, 0, 1);
			while (lastSymbol == SYM_COMMA)
			{
				set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
				set1 = uniteset(fsys, set);
				GetSymbol();
				Expression(set);
				destroyset(set1);
				destroyset(set);
				Generate(PRT, 0, 1);
			}
			if (lastSymbol == SYM_RPAREN)
			{
				GetSymbol();
			}
			else
			{
				Error(22); //missing ')'
			}
		}
	}
	else if (lastSymbol == SYM_GOTO)
	{
		GetSymbol();
		if (lastSymbol != SYM_IDENTIFIER)
		{
			Error(8);
		}
		else
		{
			GetSymbol();
			if (lastSymbol != SYM_SEMICOLON)
			{
				Error(36); //缺少';'
			}
			else //语法正确
			{
				gotoInstCount++;
				if (gotoInstCount > MAXGOTOINS)
				{
					Error(37); //goto过多
				}
				else
				{
					strcpy(gotoInstNameTab[gotoInstCount], lastIdName);
					gotoInstIndexTab[gotoInstCount] = currentInstructionIndex; //记录这个goto产生的JMP指令的地址
					Generate(JMP, 0, 0);
				}
			}
		}
	}
	Test(fsys, phi, 19);
}

//程序体
void Block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dateIndex = 3;
	block_dx = dateIndex;
	mk = (mask*)& table[tableIndex];
	mk->address = currentInstructionIndex;
	Generate(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		Error(32); // There are too many levels.
	}
	do
	{
		if (lastSymbol == SYM_CONST)
		{ // constant declarations
			GetSymbol();
			do
			{
				ConstDeclaration();
				while (lastSymbol == SYM_COMMA)
				{
					GetSymbol();
					ConstDeclaration();
				}
				if (lastSymbol == SYM_SEMICOLON)
				{
					GetSymbol();
				}
				else
				{
					Error(5); // Missing ',' or ';'.
				}
			} while (lastSymbol == SYM_IDENTIFIER);
		} // if

		if (lastSymbol == SYM_VAR)
		{ // variable declarations
			GetSymbol();
			do
			{
				Vardeclaration();
				while (lastSymbol == SYM_COMMA) //读到','
				{
					GetSymbol();
					Vardeclaration();
				}
				if (lastSymbol == SYM_SEMICOLON) //读到';'
				{
					GetSymbol();
				}
				else
				{
					Error(5); // Missing ',' or ';'.
				}
			} while (lastSymbol == SYM_IDENTIFIER);
		}			   // if
		block_dx = dateIndex; //save dateIndex before handling procedure call!
		while (lastSymbol == SYM_PROCEDURE)
		{ // procedure declarations
			GetSymbol();
			if (lastSymbol == SYM_IDENTIFIER)
			{
				Enter(ID_PROCEDURE);
				GetSymbol();
			}
			else
			{
				Error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}

			if (lastSymbol == SYM_SEMICOLON)
			{
				GetSymbol();
			}
			else
			{
				Error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tableIndex;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			Block(set);
			destroyset(set1);
			destroyset(set);
			tableIndex = savedTx;
			level--;

			if (lastSymbol == SYM_SEMICOLON)
			{
				GetSymbol();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				Test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				Error(5); // Missing ',' or ';'.
			}
		}			   // while
		dateIndex = block_dx; //restore dateIndex after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		Test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(lastSymbol, declbegsys));

	code[mk->address].a = currentInstructionIndex;
	mk->address = currentInstructionIndex;
	cx0 = currentInstructionIndex;
	Generate(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	Statement(set);
	destroyset(set1);
	destroyset(set);
	Generate(OPR, 0, OPR_RET); // return
	Test(fsys, phi, 8);	  // Test for Error: Follow the Statement is an incorrect symbol.
	ListCode(cx0, currentInstructionIndex);
}

/*
2.5 代码执行
为了简单起见，我们假设有一个PL/0 处理机，它能够解释执行PL/0 编译程
序所生成的目标代码。这个PL/0 处理机有两类存贮、一个指令寄存器和三个地
址寄存器组成。程序（目标代码）存贮称为code，由编译程序装入，在目标代
码执行过程中保持不变，因此它可被看成是“只读”存贮器。数据存贮S 组织成
为一个栈，所有的算术运算均对栈顶元和次栈顶元进行（一元运算仅作用于栈顶
元），并用结果值代替原来的运算对象。栈顶元的地址（下标）记在栈顶寄存器
T 中，指令寄存器I 包含着当前正在解释执行的指令，程序地址寄存器P 指向下
一条将取出的指令。
PL/0 的每一个过程可能包含着局部变量，因为这些过程可以被递归地调用，
故在实际调用前，无法为这些局部变量分配存贮地址。各个过程的数据区在存贮
栈S 内顺序叠起来，每个过程，除用户定义的变量外，还摇篮有它自己的内部信
息，即调用它的程序段地址（返回地址）和它的调用者的数据区地址。在过程终
止后，为了恢复原来程序的执行，这两个地址都是必须的。我们可将这两个内部
值作为位于该过程数据区的内部式隐式局部变量。我们把它们分别称为返回地址
（return address）RA 和动态链（dynamic link）DL。动态链的头，即最新分
配的数据区的地址，保存在某地址寄存器B 内。
因为实际的存贮分配是运行（解释）时进行的，编译程序不能为其生成的代
码提供绝对地址，它只能确定变量在数据区内的位置，因此它只能提供相对地址。
为了正确地存取数据，解释程序需将某个修正量加到相应的数据区的基地址上
去。若变量是局部于当前正在解释的过程，则此基地址由寄存器B 给出，否则，
就需要顺着数据区的链逐层上去找。然而遗憾的是，编译程序只能知道存取路线
表2-2 if-while 语句目标代码生成模式
12
的表态长度，同时动态链保存的则是过程活动的动态历史，而这两条存取路线并
不总是一样。
*/

//根据层次差并从当前数据区沿着静态链查找，以便获取变量实际所在的数据区其地址
int Base(int stack[], int level, int levelDiff)
{
	int b = level;

	while (levelDiff--)
		b = stack[b];
	return b;
}

//完成各种指令的执行工作
void Interpret()
{
	int pc; // program counter
	int stack[STACKSIZE];
	int top;	   // top of stack
	int b;		   // program, Base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			case OPR_NOT:
				stack[top] = !stack[top];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[Base(stack, b, i.l) + i.a];
			break;
		case LDA:
			stack[top] = stack[Base(stack, b, i.l) + stack[top] + i.a];
			break;
		case STO:
			stack[Base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case STA:
			stack[Base(stack, b, i.l) + stack[top - 1] + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top = top - 2; //此处存疑
			break;
		case CAL:
			stack[top + 1] = Base(stack, b, i.l);
			// generate new Block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case RDM:
			if (i.a == 0)
				stack[++top] = rand();
			else
				stack[++top] = rand() % i.a;
			break;
		case PRT:
			if (i.a == 0)
				printf("\n");
			else
				printf("%d\n", stack[top--]);
			break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
}

void main()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	srand((unsigned)time(0));

	lastArray.attribute = (attribute*)malloc(sizeof(attribute));

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_IDENTIFIER, SYM_GOTO, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT, SYM_RDM, SYM_NULL);

	errorCount = characterCount = currentInstructionIndex = lineLenth = 0; // initialize global variables
	lastCharacter = ' ';
	kk = MAXIDLEN;

	GetSymbol();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	Block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	for (int n = 1; n <= gotoInstCount; n++)
	{ //为每个goto产生的JMP指令回填跳转地址
		int m = label_num;
		strcpy(label_name[0], gotoInstNameTab[n]); //将goto的目标作为哨兵
		while (strcmp(label_name[m--], gotoInstNameTab[n]) != 0)
			;

		if (++m)
		{
			code[gotoInstIndexTab[n]].a = label_cx[m]; //回填
		}
		else
		{
			Error(38); //不存在这样的label
		}
	}

	if (lastSymbol != SYM_PERIOD)
		Error(9); // '.' expected.
	if (errorCount == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < currentInstructionIndex; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (errorCount == 0)
		Interpret();
	else
		printf("There are %d Error(s) in PL/0 program.\n", errorCount);
	ListCode(0, currentInstructionIndex);
}