// pl0 compiler source code

#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "PL0.h"
#include "set.c"

void expression(symset fsys);
//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= characterCount - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, errorMessage[n]);
	errorCount++;
} // error

//////////////////////////////////////////////////////////////////////
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
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (lastCharacter == ' ' || lastCharacter == '\t')
		getch();

	if (isalpha(lastCharacter))
	{ // symbol is a reserved word or an identifier.
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
			getch();
		} while (isdigit(lastCharacter));
		if (k > MAXNUMLEN)
			error(25); // The number is too great.
	}
	else if (lastCharacter == ':')
	{
		getch();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			lastSymbol = SYM_COLON; // :
		}
	}
	else if (lastCharacter == '>')
	{
		getch();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_GEQ; // >=
			getch();
		}
		else
		{
			lastSymbol = SYM_GTR; // >
		}
	}
	else if (lastCharacter == '<')
	{
		getch();
		if (lastCharacter == '=')
		{
			lastSymbol = SYM_LEQ; // <=
			getch();
		}
		else if (lastCharacter == '>')
		{
			lastSymbol = SYM_NEQ; // <>
			getch();
		}
		else
		{
			lastSymbol = SYM_LES; // <
		}
	}
	else if (lastCharacter == '&')
	{
		getch();
		if (lastCharacter == '&')
		{
			lastSymbol = SYM_AND; // &&
			getch();
		}
	}
	else if (lastCharacter == '|')
	{
		getch();
		if (lastCharacter == '|')
		{
			lastSymbol = SYM_OR; // ||
			getch();
		}
	}
	else if (lastCharacter == '!')
	{
		lastSymbol = SYM_NOT; //!
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
				if (characterCount == lineLenth) //读完本行
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
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction. //生成中间代码
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
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
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
} // test

//////////////////////////////////////////////////////////////////////
int dx; // data allocation index

// enter object(constant, variable , procedre or array) into table.//添加到符号表
void enter(int kind)
{
	mask *mk;
	arrayMask *mk_a;

	tableIndex++;
	strcpy(table[tableIndex].name, lastIdName);
	table[tableIndex].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (dimDateArray > MAXADDRESS)
		{
			error(25); // The number is too great.
			dimDateArray = 0;
		}
		table[tableIndex].value = dimDateArray;
		break;
	case ID_VARIABLE:
		mk = (mask *)&table[tableIndex];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask *)&table[tableIndex];
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
		mk_a = (arrayMask *)&table[tableIndex];
		*mk_a = lastArray;										 //至此完成name，dim，level，dimDateArray，dimSizeArray，totalSize的修改，还差address
		mk_a->attribute->address = dx;								 //dx作为首地址
		dx += mk_a->attribute->totalSize;									 //为数组开辟totalSize大小的空间
		lastArray.attribute = (attribute *)malloc(sizeof(attribute)); //先前为lastArray.attribute开辟的空间已经被table使用，开辟新的空间
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char *lastIdName)
{
	int i;
	strcpy(table[0].name, lastIdName);
	i = tableIndex + 1;
	while (strcmp(table[--i].name, lastIdName) != 0)
		;
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
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
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else
		error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration
//////////////////////////////////////////////////////////////////////
int dimConst()
{
	if (lastSymbol == SYM_IDENTIFIER || lastSymbol == SYM_NUMBER)
	{ //identifier必是const类型
		int i;
		if (lastSymbol == SYM_IDENTIFIER)
		{
			if (!(i = position(lastIdName)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				error(26); // Illegal identifier.
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
		error(28);
	}
}
//////////////////////////////////////////////////////////////////////
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
		{
			error(27); //expected ']'
		}
		else
		{
			getsym();
			dimDeclaration();
		}
	}
}

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (lastSymbol == SYM_IDENTIFIER)
	{
		getsym();
		if (lastSymbol == SYM_LBRACK)
		{ //标识符是数组
			currentArrayDim = 0;
			lastArray.kind = ID_ARRAY;
			strcpy(lastArray.name, lastIdName);
			dimDeclaration();
			enter(ID_ARRAY);
		}
		else //标识符是变量
			enter(ID_VARIABLE);
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

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void match_array_dim(symset fsys)
{ //匹配数组的维度信息，并将偏移量置于栈顶
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
			getsym();
			if (lastSymbol == SYM_LBRACK)
			{ // array
				if (!(i = position(lastIdName)))
				{
					error(11); // Undeclared identifier.
				}
				else
				{
					arrayMask *mk = (arrayMask *)&table[i];
					currentArray = *mk;
					match_array_dim(fsys);

					gen(LDA, level - mk->attribute->level, mk->attribute->address);
				}
			}
			else
			{ // variable
				if ((i = position(lastIdName)) == 0)
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
						mk = (mask *)&table[i];
						gen(LOD, level - mk->level, mk->address);
						break;
					case ID_PROCEDURE:
						error(21); // Procedure identifier can not be in an expression.
						break;
					} // switch
				}
			}
		}
		else if (lastSymbol == SYM_NUMBER)
		{
			if (dimDateArray > MAXADDRESS)
			{
				error(25); // The number is too great.
				dimDateArray = 0;
			}
			gen(LIT, 0, dimDateArray);
			getsym();
		}
		else if (lastSymbol == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (lastSymbol == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (lastSymbol == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		else if (lastSymbol == SYM_NOT)
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		else if (lastSymbol == SYM_RDM)
		{
			getsym();
			if (lastSymbol == SYM_LPAREN)
			{
				getsym();
			}
			else
				error(33);
			if (lastSymbol == SYM_RPAREN)
			{
				getsym();
				gen(RDM, 0, 0);
			}
			else if (lastSymbol == SYM_NUMBER)
			{
				getsym();
				if (lastSymbol == SYM_RPAREN)
				{
					gen(RDM, 0, dimDateArray);
					getsym();
				}
				else
					error(22);
			}
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
} // term

//////////////////////////////////////////////////////////////////////
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
} // expression

//////////////////////////////////////////////////////////////////////
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
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (inset(lastSymbol, relset))
		{
			relop = lastSymbol;
			getsym();
			set = uniteset(relset, fsys);
			expression(set);
			destroyset(set);
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
			}
			// switch
		} // else
	}	  // else
} // condition

//////////////////////////////////////////////////////////////////////
void and_condition(symset fsys)
{
	symset set, set1;

	set1 = createset(SYM_AND, SYM_NULL);
	set = uniteset(set1, fsys);
	condition(set);
	while (lastSymbol == SYM_AND)
	{
		getsym();
		condition(set);
		gen(OPR, 0, OPR_AND);
	}
	destroyset(set1);
	destroyset(set);
}

//////////////////////////////////////////////////////////////////////
void or_condition(symset fsys)
{
	symset set, set1;

	set1 = createset(SYM_OR, SYM_NULL);
	set = uniteset(set1, fsys);
	and_condition(set);
	while (lastSymbol == SYM_OR)
	{
		getsym();
		and_condition(set);
		gen(OPR, 0, OPR_OR);
	}
	destroyset(set1);
	destroyset(set);
}

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (lastSymbol == SYM_IDENTIFIER)
	{
		int cur_lever, addr;
		getsym();
		if (lastSymbol == SYM_LBRACK)
		{ // array assignment
			if (!(i = position(lastIdName)))
			{
				error(11); // Undeclared identifier.
			}

			arrayMask *mk = (arrayMask *)&table[i];
			currentArray = *mk;
			match_array_dim(fsys);

			if (lastSymbol == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}

			set = uniteset(createset(SYM_RBRACK, SYM_NULL), fsys);
			expression(fsys);
			destroyset(set);

			if (i)
				gen(STA, level - mk->attribute->level, mk->attribute->address);
		}
		else
		{ // variable assignment
			mask *mk;
			if (!(i = position(lastIdName)))
			{ //发现未定义的变量
				if (lastSymbol != SYM_COLON)
					error(11); // Undeclared identifier.
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
						error(34); //有重复的label
					}
					else
					{
						label_num++;
						if (label_num > MAXLABEL)
						{
							error(35); //label过多
						}
						else
						{
							strcpy(label_name[label_num], lastIdName);
							label_cx[label_num] = currentInstructionIndex; //存放label对应的地址
						}
						getsym();
						test(fsys, phi, 19);
						statement(fsys);
						return; //完成对 label: 的匹配
					}
				} //else 至此完成对label的处理
			}
			else if (table[i].kind != ID_VARIABLE)
			{
				error(12); // Illegal assignment.
				i = 0;
			}
			mk = (mask *)&table[i];

			if (lastSymbol == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}

			expression(fsys);

			if (i)
				gen(STO, level - mk->level, mk->address);
		}
	}
	else if (lastSymbol == SYM_CALL)
	{ // procedure call
		getsym();
		if (lastSymbol != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(lastIdName)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask *mk;
				mk = (mask *)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called.
			}
			getsym();
		}
	}
	else if (lastSymbol == SYM_IF)
	{ // if statement
		getsym();

		if (lastSymbol == SYM_LPAREN)
		{
			getsym();
		}
		else
			error(33); //Missing '('.

		set1 = createset(SYM_RPAREN, SYM_NULL);
		set = uniteset(set1, fsys);
		or_condition(set);
		destroyset(set1);
		destroyset(set);

		if (lastSymbol == SYM_RPAREN)
		{
			getsym();
		}
		else
			error(33); //Missing ')'.

		if (lastSymbol == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = currentInstructionIndex;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = currentInstructionIndex;
	}
	else if (lastSymbol == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (lastSymbol == SYM_SEMICOLON || inset(lastSymbol, statbegsys))
		{
			if (lastSymbol == SYM_SEMICOLON)
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
		if (lastSymbol == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (lastSymbol == SYM_WHILE)
	{ // while statement
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
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = currentInstructionIndex;
	}
	else if (lastSymbol == SYM_PRT)
	{ // while statement
		getsym();
		if (lastSymbol == SYM_LPAREN)
		{
			getsym();
		}
		else
			error(33);
		if (lastSymbol == SYM_RPAREN)
		{
			gen(PRT, 0, 0);
			getsym();
		}
		else
		{
			set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
			set1 = uniteset(fsys, set);
			expression(set);
			destroyset(set1);
			destroyset(set);
			gen(PRT, 0, 1);
			while (lastSymbol == SYM_COMMA)
			{
				set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
				set1 = uniteset(fsys, set);
				getsym();
				expression(set);
				destroyset(set1);
				destroyset(set);
				gen(PRT, 0, 1);
			}
			if (lastSymbol == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); //missing ')'
			}
		}
	}
	else if (lastSymbol == SYM_GOTO)
	{
		getsym();
		if (lastSymbol != SYM_IDENTIFIER)
		{
			error(8);
		}
		else
		{
			getsym();
			if (lastSymbol != SYM_SEMICOLON)
			{
				error(36); //缺少';'
			}
			else //语法正确
			{
				gotoInstCount++;
				if (gotoInstCount > MAXGOTOINS)
				{
					error(37); //goto过多
				}
				else
				{
					strcpy(gotoInstNameTab[gotoInstCount], lastIdName);
					gotoInstIndexTab[gotoInstCount] = currentInstructionIndex; //记录这个goto产生的JMP指令的地址
					gen(JMP, 0, 0);
				}
			}
		}
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
	mk = (mask *)&table[tableIndex];
	mk->address = currentInstructionIndex;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (lastSymbol == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (lastSymbol == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (lastSymbol == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (lastSymbol == SYM_IDENTIFIER);
		} // if

		if (lastSymbol == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (lastSymbol == SYM_COMMA) //读到','
				{
					getsym();
					vardeclaration();
				}
				if (lastSymbol == SYM_SEMICOLON) //读到';'
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (lastSymbol == SYM_IDENTIFIER);
		}			   // if
		block_dx = dx; //save dx before handling procedure call!
		while (lastSymbol == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (lastSymbol == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}

			if (lastSymbol == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tableIndex;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tableIndex = savedTx;
			level--;

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
	cx0 = currentInstructionIndex;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8);	  // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, currentInstructionIndex);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int level, int levelDiff)
{
	int b = level;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
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
} // interpret

//////////////////////////////////////////////////////////////////////
void main()
{
	FILE *hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	srand((unsigned)time(0));

	lastArray.attribute = (attribute *)malloc(sizeof(attribute));

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
			error(38); //不存在这样的label
		}
	}

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
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
