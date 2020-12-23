#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "PL0.h"
#include "set.c"

//打印错误信息
void error(int n)
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
void getch(void)
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
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (lastCharacter == ' ' || lastCharacter == '\t')
		getch();

	if (isalpha(lastCharacter))
	{
		//symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = lastCharacter;
			getch();
		} while (isalpha(lastCharacter) || isdigit(lastCharacter));
		a[k] = 0;
		strcpy(lastIdName, a);
		word[0] = lastIdName;
		i = NRW;
		while (strcmp(lastIdName, word[i--]));
		if (++i)
			lastSymbol = wsym[i];	 //symbol is a reserved word
		else
			lastSymbol = SYM_IDENTIFIER;	//symbol is an identifier
	}
	else if (isdigit(lastCharacter))
	{
		//symbol is a number.
		k = lastNumber = 0;
		lastSymbol = SYM_NUMBER;
		do
		{
			lastNumber = lastNumber * 10 + lastCharacter - '0';
			k++;
			getch();
		} while (isdigit(lastCharacter));
		if (k > MAXNUMLEN)
			error(25);	//The number is too great.
	}
	else if (lastCharacter == ':')
	{
		getch();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_BECOMES;	//:=
			getch();
		}
		else
			lastSymbol = SYM_NULL;	//illegal?
	}
	else if (lastCharacter == '>')
	{
		getch();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_GEQ;	//>=
			getch();
		}
		else
			lastSymbol = SYM_GTR;	//>
	}
	else if (lastCharacter == '<')
	{
		getch();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_LEQ;	//<=
			getch();
		}
		else if (lastCharacter == '>')
		{
			lastSymbol = SYM_NEQ;	//<>
			getch();
		}
		else
		{
			lastSymbol = SYM_LES;	//<
		}
	}
	else if (lastCharacter == '&')
	{
		getch();
		if (lastCharacter == '&')
		{
			lastSymbol = SYM_AND;	//&&
			getch();
		}
		else
			lastSymbol = SYM_QUOTE;		//&
	}
	else if (lastCharacter == '|')
	{
		getch();
		if (lastCharacter == '|')
		{
			lastSymbol = SYM_OR;	//||
			getch();
		}
	}
	else if (lastCharacter == '!')
	{
		lastSymbol = SYM_NOT;	//!
		getch();
	}
	else if (lastCharacter == '[')
	{
		lastSymbol = SYM_LBRACK;
		getch();
	}
	else if (lastCharacter == ']')
	{
		lastSymbol = SYM_RBRACK;
		getch();
	}
	else if (lastCharacter == '/')
		//为实现注释，将对'/'的匹配从else中删除（即删除csym与ssym中的slash）,挪到此处
	{
		getch();
		if (lastCharacter == '/') // 读到"//"
		{
			int tag = 1;
			while (tag)
			{
				getch();
				if (characterCount == lineLenth)//读完本行
				{
					tag = 0;
					getch();
				}
			}
			getsym();
		}
		else if (lastCharacter == '*') // 读到"/*"
		{
			int tag = 1;
			while (tag)
			{
				getch();
				if (lastCharacter == '*')
				{
					getch();
					if (lastCharacter == '/') //读到"*/"
					{
						tag = 0;
						getch();
					}
				}
			}
			getsym();
		}
		else
			lastSymbol = SYM_SLASH;
	}
	else if (lastCharacter == '=')
	{
		lastSymbol = SYM_ASSIGN;
		getch();
	}
	else
	{
		// other tokens
		i = NSYM;
		csym[0] = lastCharacter;
		while (csym[i--] != lastCharacter)
			;
		if (++i)
		{
			lastSymbol = ssym[i];
			getch();
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
void gen(int x, int y, int z)
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

//测试是否发生错误并跳过不属于s1或s2的所有符号
void test(symset s1, symset s2, int n)
{
	symset s;

	if (!inset(lastSymbol, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(lastSymbol, s))
			getsym();
		destroyset(s);
	}
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
的下标dx 赋初值，表示新开辟一个数据区。dx 的初值为3，因为每个数据区包
含三个内部变量RA，DL 和SL。
*/

int dx; //数据分配索引

/*
向符号表添加新的符号，并确定标识符的有关属性
常量定义：通过循环，反复获得标识符和对应的值，存入符号表。符号表中记录下标识符的名字和它对应的值。
变量定义：通过循环，反复获得标识符，存入符号表。符号表中记录下标识符的名字、它所在的层及它在所在层中的偏移地址。
*/
void enter(int kind)
{
	mask* mk;

	tabIndex++;
	strcpy(table[tabIndex].name, lastIdName);
	table[tabIndex].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (lastNumber > MAXADDRESS)
		{
			error(25); // The number is too great.
			lastNumber = 0;
		}
		table[tabIndex].value = lastNumber;
		break;
	case ID_VARIABLE:
	case ID_REFERENCE:
		mk = (mask*)& table[tabIndex];
		mk->level = currentLevel;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*)& table[tabIndex];
		mk->level = currentLevel;
		break;
		//case ID_ARRAY:
		//	if (position(lastIdName) > 0)
		//	{
		//		mk = (mask*)& table[tabIndex];
		//		mk->level = currentLevel;
		//		mk->address = dx++;
		//	}
		//	break;
	}
}

//向数组符号表添加新的数组符号，并确定标识符的有关属性
void arrayEnter()
{
	arrayTabIndex++;

	lastArray.attribute->dim = currentArrayDim;
	lastArray.attribute->dimDateArray[currentArrayDim - 1] = 1;
	lastArray.attribute->level = currentLevel;
	//计算每个维度的size
	for (int i = currentArrayDim - 1; i > 0; i--)
		lastArray.attribute->dimSizeArray[i - 1] = lastArray.attribute->dimSizeArray[i] * lastArray.attribute->dimDateArray[i];
	//计算TotalSize	 
	lastArray.attribute->totalSize = lastArray.attribute->dimSizeArray[0] * lastArray.attribute->dimDateArray[0];
	arrayTable[arrayTabIndex] = lastArray;
	//dx作为首地址
	arrayTable[arrayTabIndex].attribute->address = dx;
	//为数组开辟sum大小的空间
	dx += lastArray.attribute->totalSize;
}

//在符号表中查找标识符并返回索引
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tabIndex + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
}

//在数组符号表中查找标识符并返回索引
int arrayPosition(char* id)
{
	int i;
	strcpy(arrayTable[0].name, id);
	i = arrayTabIndex + 1;
	while (strcmp(arrayTable[--i].name, id) != 0);
	return i;
}

//常数声明
void constDeclaration()
{
	if (lastSymbol == SYM_IDENTIFIER)
	{
		getsym();
		if (lastSymbol == SYM_EQU || lastSymbol == SYM_BECOMES)
		{
			if (lastSymbol == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (lastSymbol == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
				error(2); // There must be a number to follow '='.
		}
		else
			error(3); // There must be an '=' to follow the identifier.
	}
	else
		error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
}

//维度声明 
int dimConst()
{
	if (lastSymbol == SYM_IDENTIFIER || lastSymbol == SYM_NUMBER)
	{
		//identifier必是const类型
		int i;
		if (lastSymbol == SYM_IDENTIFIER)
		{
			if (!(i = position(lastIdName)))
				error(11);	//Undeclared identifier.
			else if (table[i].kind == ID_PROCEDURE)
			{
				error(26);	//Illegal identifier.
				i = 0;
			}
			return table[i].value;
		}
		else
			return lastNumber;
	}
	else
		error(28);
	return 0;
}

//数组声明
void dimDeclaration(void)
{
	int value;
	if (lastSymbol == SYM_LBRACK)
	{
		getsym();
		value = dimConst();
		lastArray.attribute->dimDateArray[currentArrayDim++] = value;
		getsym();
		if (lastSymbol != SYM_RBRACK)
			error(27);	//expected ']'
		else
		{
			getsym();
			dimDeclaration();
		}
	}
}

//变量维度声明
void varDeclaration(void)
{
	if (lastSymbol == SYM_IDENTIFIER)
	{
		//值类型
		getsym();
		if (lastSymbol == SYM_LBRACK)
		{
			//数组
			currentArrayDim = 0;
			lastArray.kind = ID_ARRAY;
			strcpy(lastArray.name, lastIdName);
			dimDeclaration();
			arrayEnter();
		}
		else
			enter(ID_VARIABLE);
	}
	else if (lastSymbol == SYM_QUOTE)
	{
		//引用类型
		getsym();
		if (lastSymbol == SYM_IDENTIFIER)
		{
			enter(ID_REFERENCE);
			getsym();
		}
		else
			error(27);	//There must be an identifier to follow '&'.
	}
	else
		error(4);	//There must be an identifier to follow 'const', 'var', or 'procedure'.
}

/*
每一个分程序（过程）被编译结束后，将列出该部分PL/0 程序代码。
注意，每个分程序（过程）的第一条指令未被列出。
该指令是跳转指令。
其作用是绕过该分程序的说明部分所产生的代码（含过程说明所产生的代码
*/
void listcode(int from, int to)
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
void match_array_dim(symset fsys)
{
	symset set;
	currentArrayDim = 0;
	gen(LIT, 0, 0);
	while (lastSymbol == SYM_LBRACK)
	{
		currentArrayDim++;
		getsym();
		set = uniteset(createset(SYM_RBRACK, SYM_NULL), fsys);
		expression(set);
		destroyset(set);
		getsym();
		gen(LIT, 0, currentArray.attribute->dimSizeArray[currentArrayDim - 1]);
		gen(OPR, 0, OPR_MUL);
		gen(OPR, 0, OPR_ADD);
	}
}

//因子
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(lastSymbol, facbegsys))
	{
		if (lastSymbol == SYM_IDENTIFIER)
		{
			if (i = arrayPosition(lastIdName) != 0)
			{
				//数组
				getsym();
				if (lastSymbol == SYM_LBRACK)
				{
					arrayMask* mk = &arrayTable[i];
					currentArray = arrayTable[i];
					match_array_dim(fsys);
					gen(LDA, currentLevel - mk->attribute->level, mk->attribute->address);
				}
			}
			else if ((i = position(lastIdName)) == 0)
				error(11); // Undeclared identifier.
			else
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*)& table[i];
					gen(LOD, currentLevel - mk->level, mk->address);
					break;
				case ID_REFERENCE:
					mk = (mask*)& table[i];
					mk = (mask*)& table[mk->address];
					gen(LOD, currentLevel - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				case ID_ARRAY:
					//不在正常符号表内处理数组
				}
			getsym();
		}
		else if (lastSymbol == SYM_NUMBER)
		{
			if (lastNumber > MAXADDRESS)
			{
				error(25); // The number is too great.
				lastNumber = 0;
			}
			gen(LIT, 0, lastNumber);
			getsym();
		}
		else if (lastSymbol == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (lastSymbol == SYM_RPAREN)
				getsym();
			else
				error(22);	//Missing ')'.
		}
		else if (lastSymbol == SYM_MINUS)	//UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	}
}

//项
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (lastSymbol == SYM_TIMES || lastSymbol == SYM_SLASH)
	{
		mulop = lastSymbol;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
}

//表达式
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	term(set);
	while (lastSymbol == SYM_PLUS || lastSymbol == SYM_MINUS)
	{
		addop = lastSymbol;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
}

//条件
void condition(symset fsys)
{
	int relop;
	symset set;

	if (lastSymbol == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, OPR_ODD);
	}
	else if (lastSymbol == SYM_NOT)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, OPR_NOT);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (!inset(lastSymbol, relset))
		{
			error(20);
		}
		else
		{
			relop = lastSymbol;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			case SYM_AND:
				gen(OPR, 0, OPR_AND);
				break;
			case SYM_OR:
				gen(OPR, 0, OPR_OR);
				break;
			}
		}
	}
}

//语句
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (lastSymbol == SYM_IDENTIFIER)
	{
		if (i = arrayPosition(lastIdName) != 0)
		{
			//数组
			arrayMask* amk;
			getsym();
			if (lastSymbol == SYM_LBRACK)
			{
				arrayMask* amk = &arrayTable[i];
				currentArray = arrayTable[i];
				match_array_dim(fsys);
				if (lastSymbol == SYM_BECOMES)
					getsym();
				else
					error(13); // ':=' expected.
				set = uniteset(createset(SYM_RBRACK, SYM_NULL), fsys);
				expression(fsys);
				destroyset(set);
				if (i)
					gen(STA, currentLevel - amk->attribute->level, amk->attribute->address);
			}
		}
		else
		{
			//变量
			mask* mk;
			if (!(i = position(lastIdName)))
				error(11);	 //Undeclared identifier.
			else if (table[i].kind != ID_VARIABLE && table[i].kind != ID_REFERENCE)
			{
				error(12);	//Illegal assignment.
				i = 0;
			}
			getsym();
			if (lastSymbol == SYM_BECOMES)
				getsym();
			else
				error(13);	//':='expected.
			if (table[i].kind == ID_VARIABLE)
			{
				expression(fsys);
				mk = (mask*)& table[i];
				if (i)
					gen(STO, currentLevel - mk->level, mk->address);
			}
			else if (table[i].kind == ID_REFERENCE)
			{
				mk = (mask*)& table[i];
				if (lastSymbol == SYM_IDENTIFIER)
					if ((i = position(lastIdName)) == 0)
						error(11); // Undeclared identifier.
					else
						if (table[i].kind != ID_VARIABLE)
							error(28);	//The reference must be assigned by an identifier.
						else
						{
							gen(LIT, 0, position(lastIdName));
							gen(STO, currentLevel - mk->level, mk->address);
							getsym();
						}
				else
					error(28);	//The reference must be assigned by an identifier.
			}
		}
	}
	else if (lastSymbol == SYM_CALL)
	{
		//procedure call
		getsym();
		if (lastSymbol != SYM_IDENTIFIER)
			error(14);	//There must be an identifier to follow the 'call'.
		else
		{
			if (!(i = position(lastIdName)))
				error(11);	//Undeclared identifier.
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*)& table[i];
				gen(CAL, currentLevel - mk->level, mk->address);
			}
			else
				error(15);	//A constant or variable can not be called.
			getsym();
		}
	}
	else if (lastSymbol == SYM_IF)
	{
		// if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (lastSymbol == SYM_THEN)
			getsym();
		else
			error(16);	//'then' expected.
		cx1 = currentInstructionIndex;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = currentInstructionIndex;
	}
	else if (lastSymbol == SYM_BEGIN)
	{
		//block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (lastSymbol == SYM_SEMICOLON || inset(lastSymbol, statbegsys))
		{
			if (lastSymbol == SYM_SEMICOLON)
				getsym();
			else
				error(10);
			statement(set);
		}
		destroyset(set1);
		destroyset(set);
		if (lastSymbol == SYM_END)
			getsym();
		else
			error(17);	//';' or 'end' expected.
	}
	else if (lastSymbol == SYM_WHILE)
	{
		// while statement
		cx1 = currentInstructionIndex;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = currentInstructionIndex;
		gen(JPC, 0, 0);
		if (lastSymbol == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18);	//'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = currentInstructionIndex;
	}
	test(fsys, phi, 19);
}

//程序体
void block(symset fsys)
{
	int codeIndex;
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*)& table[tabIndex];
	mk->address = currentInstructionIndex;
	gen(JMP, 0, 0);
	if (currentLevel > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (lastSymbol == SYM_CONST)
		{
			//constant declarations
			getsym();
			do
			{
				constDeclaration();
				while (lastSymbol == SYM_COMMA)
				{
					getsym();
					constDeclaration();
				}
				if (lastSymbol == SYM_SEMICOLON)
					getsym();
				else
					error(5); // Missing ',' or ';'.
			} while (lastSymbol == SYM_IDENTIFIER);
		}
		if (lastSymbol == SYM_VAR)
		{
			//variable declarations
			getsym();
			do
			{
				varDeclaration();
				while (lastSymbol == SYM_COMMA)		//读到','
				{
					getsym();
					varDeclaration();
				}
				if (lastSymbol == SYM_SEMICOLON)	//读到';'
					getsym();
				else
					error(5);	//Missing ',' or ';'.
			} while (lastSymbol == SYM_IDENTIFIER);
		}
		block_dx = dx; //save dx before handling procedure call!
		while (lastSymbol == SYM_PROCEDURE)
		{
			//procedure declarations
			getsym();
			if (lastSymbol == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
				error(4);	//There must be an identifier to follow 'const', 'var', or 'procedure'.
			if (lastSymbol == SYM_SEMICOLON)
				getsym();
			else
				error(5);	//Missing ',' or ';'.

			currentLevel++;
			savedTx = tabIndex;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tabIndex = savedTx;
			currentLevel--;

			if (lastSymbol == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		}			   // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(lastSymbol, declbegsys));

	code[mk->address].a = currentInstructionIndex;
	mk->address = currentInstructionIndex;
	codeIndex = currentInstructionIndex;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8);	  // test for error: Follow the statement is an incorrect symbol.
	listcode(codeIndex, currentInstructionIndex);
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
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
}

//完成各种指令的执行工作
void interpret()
{
	int pc; // program counter
	int stack[STACKSIZE];
	int top;	   // top of stack
	int b;		   // program, base, and top-stack register
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
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case LDA:
			stack[top] = stack[base(stack, b, i.l) + stack[top] + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case STA:
			stack[base(stack, b, i.l) + stack[top - 1] + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top = top - 2; //此处存疑
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
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

	lastArray.attribute = (arrayAttribute*)malloc(sizeof(arrayAttribute));

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_AND, SYM_OR, SYM_NOT, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);

	errorCount = characterCount = currentInstructionIndex = lineLenth = 0; // initialize global variables
	lastCharacter = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (lastSymbol != SYM_PERIOD)
		error(9); // '.' expected.
	if (errorCount == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < currentInstructionIndex; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (errorCount == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", errorCount);
	listcode(0, currentInstructionIndex);
}