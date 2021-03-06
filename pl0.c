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
		while ((!feof(infile)) // added & modified by alex 01-02-09
			   && ((ch = getc(infile)) != '\n'))
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
int pre_sym_count = 0;
int sym_stack[10] = {0};

void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];
	if (pre_sym_count > 0)
	{
		sym = sym_stack[--pre_sym_count];
		return;
	}
	while (ch == ' ' || ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]))
			;
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER; // symbol is an identifier
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
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25); // The number is too great.
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
			sym = SYM_COLON; // :
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ; // >=
			getch();
		}
		else
		{
			sym = SYM_GTR; // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ; // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ; // <>
			getch();
		}
		else
		{
			sym = SYM_LES; // <
		}
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND; // &&
			getch();
		}
		else
			sym = SYM_QUOTE;
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
				if (cc == ll) //读完本行
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
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch)
			;
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

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction. //生成中间代码
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

	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
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
	mask_array *mk_a;

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
		mk = (mask *)&table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask *)&table[tx];
		mk->level = now_procedure;
		all_procedure[now_procedure].next = NULL;  //初始化函数表中结点
		all_procedure[now_procedure].para_num = 0; //
		break;
	case ID_ARRAY:
		lastArray.attr->dim = cur_dim;
		lastArray.attr->size[cur_dim - 1] = 1;
		lastArray.attr->level = level;
		for (int i = cur_dim - 1; i > 0; i--)
		{
			lastArray.attr->size[i - 1] = lastArray.attr->size[i] * lastArray.attr->num[i]; //计算每个维度的size
		}
		lastArray.attr->sum = lastArray.attr->size[0] * lastArray.attr->num[0]; //计算sum
		mk_a = (mask_array *)&table[tx];
		*mk_a = lastArray;										 //至此完成name，dim，level，num，size，sum的修改，还差address
		mk_a->attr->address = dx;								 //dx作为首地址
		dx += mk_a->attr->sum;									 //为数组开辟sum大小的空间
		lastArray.attr = (attribute *)malloc(sizeof(attribute)); //先前为lastArray.attr开辟的空间已经被table使用，开辟新的空间
		break;
	case ID_PARAMETER_A:
		lastArray.attr->dim = cur_dim;
		lastArray.attr->size[cur_dim - 1] = 1;
		lastArray.attr->level = 0; //参数只会用主程序中变量
		for (int i = cur_dim - 1; i > 0; i--)
		{
			lastArray.attr->size[i - 1] = lastArray.attr->size[i] * lastArray.attr->num[i]; //计算每个维度的size
		}
		lastArray.attr->sum = lastArray.attr->size[0] * lastArray.attr->num[0]; //计算sum
		mk_a = (mask_array *)&table[tx];
		*mk_a = lastArray;			//至此完成name，dim，level，num，size，sum的修改，还差address
		mk_a->attr->address = dx++; //dx作为首地址
		//dx += mk_a->attr->sum;//参数只分配一个空间									 //为数组开辟sum大小的空间
		lastArray.attr = (attribute *)malloc(sizeof(attribute)); //先前为lastArray.attr开辟的空间已经被table使用，开辟新的空间
		break;
	case ID_REFERENCE:
		mk = (mask *)&table[tx];
		mk->level = level;
		break;
	case ID_PARAMETER_I: //引用参数变量
		mk = (mask *)&table[tx];
		mk->level = 0;
		mk->address = dx++;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char *id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0)
		;
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
	}
	else
		error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration
//////////////////////////////////////////////////////////////////////
int dimConst()
{
	if (sym == SYM_IDENTIFIER || sym == SYM_NUMBER)
	{ //identifier必是const类型
		int i;
		if (sym == SYM_IDENTIFIER)
		{
			if (!(i = position(id)))
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
			return num;
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
	if (sym == SYM_LBRACK)
	{
		getsym();
		value = dimConst();

		lastArray.attr->num[cur_dim++] = value;

		getsym();
		if (sym != SYM_RBRACK)
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
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_LBRACK)
		{ //标识符是数组
			cur_dim = 0;
			lastArray.kind = ID_ARRAY;
			strcpy(lastArray.name, id);
			dimDeclaration();
			enter(ID_ARRAY);
		}
		else //标识符是变量
			enter(ID_VARIABLE);
	}
	else if (sym == SYM_QUOTE)
	{ //引用变量
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			enter(ID_REFERENCE);
			getsym();
			if (sym == SYM_EQU)
			{
				getsym();
				if (sym == SYM_IDENTIFIER)
				{
					getsym();
					int i = position(id);
					if (i != 0)
					{
						mask *mk1 = (mask *)&table[tx];				//引用变量的符号表
						mask_array *mk2 = (mask_array *)&table[tx]; //引用变量的符号表
						mask *mk3 = (mask *)&table[i];				//被引用变量的符号表
						mask_array *mk4 = (mask_array *)&table[i];	//被引用变量的符号表
						switch (table[i].kind)
						{
						case ID_VARIABLE:
							mk1->kind = ID_VARIABLE;
							mk1->address = mk3->address;
							break;
						case ID_ARRAY:
							if (sym == SYM_LBRACK)
							{ //引用数组中的某个元素
								cur_dim = 0;
								dimDeclaration(); //获取引用的元素的目标
								mk1->kind = ID_VARIABLE;
								mk1->address = mk4->attr->address;
								int k;
								for (k = 0; k < mk4->attr->dim; k++)
								{
									mk1->address += lastArray.attr->num[k] * mk4->attr->size[k];
								}
							}
							else
							{ //直接引用数组名
								mk2->kind = ID_ARRAY;
								mk2->attr = mk4->attr;
							}
							break;
						default:
							break;
						}
					}
					else
						error(11); //Undeclared identifier.
				}
				else
					error(31); //There must be a identify to follow '='.
			}
			else
				error(30); //The reference does not initial.
		}
		else
			error(29); //There must be an identify to follow '&'.
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
void match_array_dim(void)
{ //匹配数组的维度信息，并将偏移量置于栈顶
	symset set;
	cur_dim = 0;
	gen(LIT, 0, 0); //存放偏移量
	while (sym == SYM_LBRACK)
	{
		cur_dim++;
		getsym();
		set = createset(SYM_RBRACK, SYM_NULL);
		expression(set);
		destroyset(set);
		getsym();
		gen(LIT, 0, curArray.attr->size[cur_dim - 1]);
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

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			getsym();
			if (sym == SYM_LBRACK)
			{ // array
				if (!(i = position(id)))
				{
					error(11); // Undeclared identifier.
				}
				else
				{
					mask_array *mk = (mask_array *)&table[i];
					int kind = mk->kind;
					curArray = *mk;
					match_array_dim();

					if (kind == ID_ARRAY)
						gen(LDA, level - mk->attr->level, mk->attr->address);
					else if (kind == ID_PARAMETER_A)
					{
						gen(LOD, 0, mk->attr->address);
						gen(LDP, 1, 0); //值与level-mk->attr->level相同
					}
				}
			}
			else
			{ // variable
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
					case ID_REFERENCE:
					case ID_VARIABLE:
						mk = (mask *)&table[i];
						gen(LOD, level - mk->level, mk->address);
						break;
					case ID_PROCEDURE:
						error(21); // Procedure identifier can not be in an expression.
						break;
					case ID_PARAMETER_I:
						mk = (mask *)&table[i];
						gen(LOD, 0, mk->address);
						gen(LDA, 1, 0); //认为只会出现层次差为 1 的情况
						break;
					} // switch
				}
			}
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
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NOT)
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		else if (sym == SYM_RDM)
		{
			getsym();
			if (sym == SYM_LPAREN)
			{
				getsym();
			}
			else
				error(33);
			if (sym == SYM_RPAREN)
			{
				getsym();
				gen(RDM, 0, 0);
			}
			else if (sym == SYM_NUMBER)
			{
				getsym();
				if (sym == SYM_RPAREN)
				{
					gen(RDM, 0, num);
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

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

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
		gen(OPR, 0, OPR_ODD);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (inset(sym, relset))
		{
			relop = sym;
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
	while (sym == SYM_AND)
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
	while (sym == SYM_OR)
	{
		getsym();
		and_condition(set);
		gen(OPR, 0, OPR_OR);
	}
	destroyset(set1);
	destroyset(set);
}

//////////////////////////////////////////////////////////////////////
void call(int i, symset fsys)
{
	int para_num, j, k = 0;
	mask *mk_p = (mask *)&table[i];
	mask *mk;
	mask_array *mk_a;
	symset set;
	now_procedure = mk_p->level;
	para_num = all_procedure[now_procedure].para_num;
	procedure_parameter *parameter = all_procedure[now_procedure].next;

	getsym();
	if (sym == SYM_LPAREN) //左括号
	{
		getsym();
		for (k = 0; k < para_num && sym != SYM_RPAREN; k++)
		{
			int kind = parameter->kind;
			switch (kind)
			{
			case ID_VARIABLE:
				set = uniteset(createset(SYM_RBRACK, SYM_COMMA, SYM_NULL), fsys);
				expression(set);
				destroyset(set);
				break;
			case ID_PARAMETER_I:
				if (!(j = position(id)))
				{
					error(11); // Undeclared identifier.
				}
				if (table[j].kind == ID_VARIABLE)
				{
					mk = (mask *)&table[j];
					gen(LIT, 0, mk->address);
					getsym();
				}
				else if (table[j].kind == ID_ARRAY)
				{
					getsym();
					if (sym == SYM_LBRACK)
					{
						mk_a = (mask_array *)&table[j];
						curArray = *mk_a;
						match_array_dim();
						gen(LIT, 0, mk_a->attr->address);
						gen(OPR, 0, OPR_ADD);
					}
					else
					{
						error(39); //expecting '['
					}
				}
				else
				{
					error(40); //  参数格式不匹配
				}
				break;
			case ID_PARAMETER_A:
				if (!(j = position(id)))
				{
					error(11); // Undeclared identifier.
				}
				if (table[j].kind != ID_ARRAY)
				{
					error(40); //  参数格式不匹配
				}
				else
				{
					mk_a = (mask_array *)&table[j];
					gen(LIT, 0, mk_a->attr->address);
					getsym();
					break;
				}
			}
			if (sym == SYM_COMMA)
			{
				getsym();
			}
			else if (sym != SYM_RPAREN)
			{
				error(43); // missing ','
			}
			parameter = parameter->next;
		}
		if (sym == SYM_RPAREN && k == para_num) //右括号
		{
			gen(PAS, 0, para_num);
			gen(CAL, 0, mk_p->address);
		}
		else
		{
			error(41); // 参数传递错误
		}
	}
	else
	{
		error(33); // missing '('
	}
}

void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{
		int cur_lever, addr, kind;
		getsym();
		if (sym == SYM_LBRACK)
		{ // array assignment
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}

			mask_array *mk = (mask_array *)&table[i];
			kind = mk->kind;
			curArray = *mk;
			match_array_dim();

			if (sym == SYM_BECOMES)
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

			if (kind == ID_ARRAY)
				gen(STA, level - mk->attr->level, mk->attr->address);
			else if (kind == ID_PARAMETER_A)
			{
				gen(LOD, 0, mk->attr->address); //取出传进的数组在主活动记录的偏移
				gen(STP, 1, 0);					//i.a用不到，i.l必为1，与level - mk->attr->level值相同
			}
		}
		else
		{ // variable assignment
			mask *mk;
			if (!(i = position(id)))
			{ //发现未定义的变量
				if (sym != SYM_COLON)
					error(11); // Undeclared identifier.
				else		   //id是一个label
				{			   //开始对label的处理
					strcpy(label_name[0], id);
					int k = label_num;
					while (strcmp(label_name[k--], id) != 0) //检查是否有重复的label
						;

					if (++k)
					{
						error(34); //有重复的label
					}
					else
					{
						label_num++;
						if (label_num > NLABEL)
						{
							error(35); //label过多
						}
						else
						{
							strcpy(label_name[label_num], id);
							label_cx[label_num] = cx; //存放label对应的地址
						}
						getsym();
						statement(fsys);
						return; //完成对 label: 的匹配
					}
				} //else 至此完成对label的处理
			}
			else if (table[i].kind != ID_VARIABLE && table[i].kind != ID_PARAMETER_I)
			{
				error(12); // Illegal assignment.
				i = 0;
			}
			mk = (mask *)&table[i];
			kind = mk->kind;

			if (sym == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}

			expression(fsys);

			if (kind == ID_VARIABLE)
				gen(STO, level - mk->level, mk->address);
			else if (kind == ID_PARAMETER_I)
			{
				gen(LOD, 0, mk->address);
				gen(STI, 1, 0); //参数调用认为层差为 1，不用 i.a
			}
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
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				//mask *mk;
				//mk = (mask *)&table[i];
				//gen(CAL, level - mk->level, mk->address);
				call(i, fsys);
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

		if (sym == SYM_LPAREN)
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

		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
			error(33); //Missing ')'.

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

		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else
		{
			error(10);
		}

		if (sym == SYM_ELSE)
		{
			int cx2 = cx;
			gen(JMP, 0, 0);
			code[cx1].a = cx;
			getsym();
			statement(fsys);
			code[cx2].a = cx;
		}
		else
		{
			sym_stack[pre_sym_count++] = sym;
			sym = SYM_SEMICOLON;
			code[cx1].a = cx;
		}
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

		if (sym == SYM_LPAREN)
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

		cx2 = cx;
		gen(JPC, 0, 0);

		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
			error(33); //Missing ')'.

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
	else if (sym == SYM_PRT)
	{ // while statement
		getsym();
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		else
			error(33);
		if (sym == SYM_RPAREN)
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
			while (sym == SYM_COMMA)
			{
				set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
				set1 = uniteset(fsys, set);
				getsym();
				expression(set);
				destroyset(set1);
				destroyset(set);
				gen(PRT, 0, 1);
			}
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); //missing ')'
			}
		}
	}
	else if (sym == SYM_GOTO)
	{
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(8);
		}
		else
		{
			getsym();
			if (sym != SYM_SEMICOLON)
			{
				error(36); //缺少';'
			}
			else //语法正确
			{
				goto_num++;
				if (goto_num > NGOTO)
				{
					error(37); //goto过多
				}
				else
				{
					strcpy(goto_dest[goto_num], id);
					goto_cx[goto_num] = cx; //记录这个goto产生的JMP指令的地址
					gen(JMP, 0, 0);
				}
			}
		}
	}
	test(fsys, phi, 19);
} // statement

void enter_parameter(int kind) //向相应函数表项下增加参数结点
{
	int i = all_procedure[now_procedure].para_num;
	procedure_parameter *new_para = (procedure_parameter *)malloc(sizeof(procedure_parameter));
	new_para->kind = kind;
	new_para->next = NULL;
	if (i)
	{
		procedure_parameter *p = all_procedure[now_procedure].next;
		while (--i)
		{
			p = p->next;
		}
		p->next = new_para;
	}
	else
	{
		all_procedure[now_procedure].next = new_para;
	}
	all_procedure[now_procedure].para_num++;
}

//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask *mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3 + parameter_num;
	block_dx = dx;
	mk = (mask *)&table[tx - parameter_num]; //还原函数在符号表中位置
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
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA) //读到','
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON) //读到';'
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		}			   // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE && level == 0)
		{ // procedure declarations 大改
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

			level++;
			parameter_num = 0;
			dx = 3;
			savedTx = tx;
			if (sym == SYM_LPAREN)
			{
				getsym();
				while (sym == SYM_VAR)
				{
					getsym();
					if (sym == SYM_IDENTIFIER)
					{
						getsym();
						if (sym == SYM_LBRACK)
						{ //标识符是数组
							cur_dim = 0;
							lastArray.kind = ID_PARAMETER_A;
							strcpy(lastArray.name, id);
							dimDeclaration();
							enter(ID_PARAMETER_A);
							enter_parameter(ID_PARAMETER_A);
						}
						else //标识符是变量
						{
							enter(ID_VARIABLE);
							enter_parameter(ID_VARIABLE);
						}
						parameter_num++;
					}
					else if (sym == SYM_QUOTE) //标识符是引用
					{
						getsym();
						if (sym == SYM_IDENTIFIER)
						{
							enter(ID_PARAMETER_I);
							enter_parameter(ID_PARAMETER_I);
							getsym();
							parameter_num++;
						}
					}
					else
					{
						error(42); //待修改 丢失“&”或id
					}
					if (sym == SYM_COMMA)
					{
						getsym();
					}
				}
				if (sym == SYM_RPAREN)
				{
					getsym();
				}
				else
				{
					error(22); // Missing ')'
				}
			}
			else
			{
				error(33); // Missing '('.
			}
			now_procedure++;
			//level++;
			//savedTx = tx;
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
		}			   // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

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
	test(fsys, phi, 8);	  // test for error: Follow the statement is an incorrect symbol.
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
	int pc; // program counter
	int stack[STACKSIZE];
	int top;	   // top of stack
	int b;		   // program, base, and top-stack register
	instruction i; // instruction register

	int k;

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
		case LDA: //使用栈顶存放的偏移量
			stack[top] = stack[base(stack, b, i.l) + stack[top] + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case STA: //使用次栈顶存放的偏移量
			stack[base(stack, b, i.l) + stack[top - 1] + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top = top - 2;
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
		case PAS:
			for (k = i.a; k > 0; k--)
			{
				stack[top + 3] = stack[top];
				top--;
			}
			break;
		case LDP: //LDA改编，i.a从栈顶取
			top--;
			stack[top] = stack[base(stack, b, i.l) + stack[top] + stack[top + 1]]; //活动记录基址+数组内偏移+数组首地址偏移
			break;
		case STP: //同理
			stack[base(stack, b, i.l) + stack[top - 2] + stack[top]] = stack[top - 1];
			printf("%d\n", stack[top - 1]);
			top -= 3;
			break;
		case STI:
			stack[base(stack, b, i.l) + stack[top]] = stack[top - 1];
			printf("%d\n", stack[top - 1]);
			top -= 2;
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

	lastArray.attr = (attribute *)malloc(sizeof(attribute));
	parameter_num = 0;
	now_procedure = 0;

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
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_GOTO, SYM_NULL); //此处不应有SYM_IDENTIFIER
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NOT, SYM_RDM, SYM_NULL);

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

	for (int n = 1; n <= goto_num; n++)
	{ //为每个goto产生的JMP指令回填跳转地址
		int m = label_num;
		strcpy(label_name[0], goto_dest[n]); //将goto的目标作为哨兵
		while (strcmp(label_name[m--], goto_dest[n]) != 0)
			;

		if (++m)
		{
			code[goto_cx[n]].a = label_cx[m]; //回填
		}
		else
		{
			error(38); //不存在这样的label
		}
	}

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
