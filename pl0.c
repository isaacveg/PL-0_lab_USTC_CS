// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
<<<<<<< HEAD
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
=======
		while ((!feof(infile)) // added & modified by alex 01-02-09
			   && ((ch = getc(infile)) != '\n'))
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '&')
	{
<<<<<<< HEAD
        getch();
        if (ch == '&')
        {
            sym = SYM_AND;     // &&
            getch();
        }

	}
	else if (ch == '|')
    {
        getch();
        if (ch == '|')
        {
            sym = SYM_OR;     // &&
            getch();
        }
    }
    else if (ch == '!')
    {
        sym = SYM_NOT;
        getch();
    }
=======
		getch();
		if (ch == '&')
		{
			sym = SYM_AND; // &&
			getch();
		}
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR; // ||
			getch();
		}
	}
	else if (ch == '!')
	{
		sym = SYM_NOT; //!
		getch();
	}
	else if (ch == '[')
	{
		sym = SYM_LBRACK;
		getch();
	}
	else if (ch == ']')
	{
		sym = SYM_RBRACK;
		getch();
	}
	else if (ch == '/')
	//为实现注释，将对'/'的匹配从else中删除（即删除csym与ssym中的slash）,挪到此处
	{
		getch();
		if (ch == '/') // 读到"//"
		{
			int tag = 1;
			while (tag)
			{
				getch();
				if (cc == ll)//读完本行
				{
					tag = 0;
					getch();
				}
			}
			getsym();
		}
		else if (ch == '*') // 读到"/*"
		{
			int tag = 1;
			while (tag)
			{
				getch();
				if (ch == '*')
				{
					getch();
					if (ch == '/') //读到"*/"
					{
						tag = 0;
						getch();
					}
				}
			}
			getsym();
		}
		else
		{
			sym = SYM_SLASH;
		}
	}
>>>>>>> parent of f50d058... 代码格式化
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

<<<<<<< HEAD
//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
=======
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
>>>>>>> parent of f50d058... 代码格式化
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask *mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
		mk = (mask*) &table[tx];
=======
		mk = (mask *)&table[tx];
>>>>>>> parent of f50d058... 代码格式化
=======
		mk = (mask *)&table[tx];
>>>>>>> parent of f50d058... 代码格式化
=======
		mk = (mask *)&table[tx];
>>>>>>> parent of f50d058... 代码格式化
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
=======
=======
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
		mk = (mask *)&table[tx];
		mk->level = level;
		break;
	case ID_ARRAY: /*********************/
		/*code*/
		break;
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
	} // switch
} // enter

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
=======
//在符号表中查找标识符
int position(char *id)
>>>>>>> parent of f50d058... 代码格式化
=======
//在符号表中查找标识符
int position(char *id)
>>>>>>> parent of f50d058... 代码格式化
=======
//在符号表中查找标识符
int position(char *id)
>>>>>>> parent of f50d058... 代码格式化
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
<<<<<<< HEAD
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

=======
	}
	else
		error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
}

int dim;//声明数组的维度

void dimDeclaration(void)
{
	dim++;
	int i;
	if (sym == SYM_IDENTIFIER || sym == SYM_NUMBER)
	{ //如何enter 如何组织记录一个数组

		/*
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_PROCEDURE)
		{
			error(26); // Illegal assignment.
			i = 0;
		}
		*/
	}
}

<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
<<<<<<< HEAD
=======
		if (sym == SYM_LBRACK)
		{
			getsym();
			dim = 0;
			dimDeclaration();
		}
		else
			enter(ID_VARIABLE);
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	
=======

>>>>>>> parent of f50d058... 代码格式化
=======

>>>>>>> parent of f50d058... 代码格式化
=======

>>>>>>> parent of f50d058... 代码格式化
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask *mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
					mk = (mask*) &table[i];
=======
					mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
=======
					mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
=======
					mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
        else if(sym == SYM_NOT) // NOT,  Expr -> '!' Expr
        {
            getsym();
            factor(fsys);
            gen(OPR, 0, OPR_NOT);
        }
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
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
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL, SYM_NOT));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
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
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;


	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		set = uniteset(set,createset(SYM_NOT));
		expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
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
			} // switch
		} // else
	} // else
} // condition

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
		mask* mk;
		if (! (i = position(id)))
=======
=======
>>>>>>> parent of f50d058... 代码格式化
=======
>>>>>>> parent of f50d058... 代码格式化
		mask *mk;
		if (!(i = position(id)))
>>>>>>> parent of f50d058... 代码格式化
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
		mk = (mask*) &table[i];
=======
		mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
=======
		mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
=======
		mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
				mask* mk;
				mk = (mask*) &table[i];
=======
				mask *mk;
				mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
=======
				mask *mk;
				mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
=======
				mask *mk;
				mk = (mask *)&table[i];
>>>>>>> parent of f50d058... 代码格式化
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;	
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask *mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	mk = (mask*) &table[tx];
=======
	mk = (mask *)&table[tx];
>>>>>>> parent of f50d058... 代码格式化
=======
	mk = (mask *)&table[tx];
>>>>>>> parent of f50d058... 代码格式化
=======
	mk = (mask *)&table[tx];
>>>>>>> parent of f50d058... 代码格式化
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
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
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base
	

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
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
			case OPR_NOT:
				stack[top] = !stack[top];
                break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
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
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE *hbin;
	char s[80];
	int i;
	symset set, set1, set2;

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
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT,SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
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

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
