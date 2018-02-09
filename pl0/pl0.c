// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "pl0.h"
#include "set.h"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)//输出错误类型，n为错误类型在错误信息数组中对应的字符串的索引
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
	if (cc == ll)//cc是表示行缓冲区位置，ll是行缓冲区长度
	{
		if (feof(infile))    //
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))   //读取文件的一整行
		{
			printf("%c", ch);                       //打印读出的字符
			if (ch == '/' &&line[ll] == '/')
			{
				ll--;
				while ((ch = getc(infile)) != '\n')
					printf("%c", ch);
				break;
			}
			else if (ch == '*'&&line[ll] == '/')
			{
				ll--;
				while (1)
				{
					ch = getc(infile);
					printf("%c", ch);
					if (ch == '\n')
						printf("%7c", ' ');
					else if (ch == '*' && (ch = getc(infile)) == '/')
					{
						printf("%c",ch);
						break;
					}
				}
				continue;
			}
			    line[++ll] = ch;                        //将字符保存到行缓冲区
		} // while
		printf("\n");                               //打印完一整行之后换行
		line[++ll] = ' ';                           //行末设置为空字符
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
	{ // symbol is a reserved word or an identifier. or a goto chance
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
		} while (isdigit(ch));
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
			sym = SYM_COLON;     // :
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
		else if(ch == '>')     //>>
        {
            sym = SYM_RS;
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
		else if(ch == '<')
        {
            sym = SYM_LS;
            getch();
        }

		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else
		{
			sym = SYM_OP_AND;
		}
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		else
		{
			sym = SYM_OP_OR;
		}
	}
	else if (ch == '!')
	{
		getch();
		sym = SYM_NOT;

	}
	else if(ch == '[')
    {
        getch();
        sym = SYM_DIMSTART;
    }
    else if(ch == ']')
    {
        getch();
        sym = SYM_DIMEND;
    }//dim tokens


	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
			if (sym == SYM_PLUS && ch == '+')
			{
				getch();
				sym = SYM_INC;
			}
			else if (sym == SYM_MINUS && ch == '-')
			{
				getch();
				sym = SYM_DEC;
			}
			else if (sym == SYM_PLUS && ch == '=')
            {
                getch();
                sym = SYM_ADDBECOMES;
            }
			else if (sym == SYM_MINUS && ch == '=')
            {
                getch();
                sym = SYM_MINBECOMES;
            }
			else if (sym == SYM_TIMES && ch == '=')
            {
                getch();
                sym = SYM_MULBECOMES;
            }
			else if (sym == SYM_SLASH && ch == '=')
            {
                getch();
                sym = SYM_DIVBECOMES;
            }
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
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
	mask* mk;
	if(position(id)>0)
	{
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
	}
	else{
	
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
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;

	}// switch
}
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
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
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void arraydeclaration(void)
{
	int i;
}//deal with array



void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
	    if(sym == SYM_DIMSTART)
	    arraydeclaration();
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


void addr(mask*mk,symset fsys)
{
	void expression(symset fsys);
	int i;

	
	
	
}
//address the element of array
void call(int i, symset fsys);
//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if(inset(sym, facbegsys))
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
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					getsym();
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					getsym();
					if(sym == SYM_DIMSTART)//array element , deal with address
                    {
                        addr(mk,fsys);
					}
					if (sym == SYM_INC)
					{
						gen(LOD, level - mk->level, mk->address);
						gen(INC, level - mk->level, mk->address);
						getsym();
					}
					else if (sym == SYM_DEC)
					{
						gen(LOD, level - mk->level, mk->address);
						gen(DEC, level - mk->level, mk->address);
						getsym();
					}
					else
                    {
						if(sym == SYM_BECOMES)
                        {
                            getsym();
                            expression(fsys);
                            gen(STO, level - mk->level, mk->address);
                        }
						gen(LOD, level - mk->level, mk->address);
                    }
					break;
				case ID_PROCEDURE:
                        call(i, fsys);
					break;
				} // switch
			}
			//getsym();
		}
		else if (sym == SYM_INC)
		{
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				if ((i = position(id)) == 0)
				{
					error(11); // Undeclared identifier.
				}
				else
				{
					if (table[i].kind == ID_VARIABLE)
					{
						mask *mk;
						mk = (mask *)&table[i];
						if(sym == SYM_DIMSTART)//array element , deal with address
                    {
                        addr(mk,fsys);
					}
						gen(INC, level - mk->level, mk->address);
						gen(LOD, level - mk->level, mk->address);
					}
					else
						error(12);
				}
			}
			else
				error(14);
			getsym();
		}
		else if (sym == SYM_DEC)
		{
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				if ((i = position(id)) == 0)
				{
					error(11); // Undeclared identifier.
				}
				else
				{
					if (table[i].kind == ID_VARIABLE)
					{
						mask *mk;
						mk = (mask *)&table[i];
						if(sym == SYM_DIMSTART)//array element , deal with address
                    {
                        addr(mk,fsys);
					}
						gen(DEC, level - mk->level, mk->address);
						gen(LOD, level - mk->level, mk->address);
					}
					else
						error(12);
				}
			}
			else
				error(14);
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
			 expression(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		else if(sym ==SYM_RDM)
        {
            getsym();
            if(sym==SYM_LPAREN)
            {
                getsym();
                if(sym==SYM_NUMBER)
                {
                    gen(RDM, 0, num);
                    getsym();
                    if(sym==SYM_RPAREN)
                        getsym();
                    else error(22);
                }
                else if(sym==SYM_RPAREN)
                {
                    gen(RDM, 0,0);
                    getsym();
                }
                else error(22);
            }
        }
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
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
void expression2(symset fsys)
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

void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_LS, SYM_RS, SYM_NULL));

	expression2(set);
	while (sym == SYM_LS || sym == SYM_RS)
	{
		addop = sym;
		getsym();
		expression2(set);
		if (addop == SYM_LS)
		{
			gen(OPR, 0, OPR_LS);
		}
		else
		{
			gen(OPR, 0, OPR_RS);
		}
	} // while
	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys);//声明
int condition_1(symset fsys)//条件分析处理
{
    int relop;//用于临时记录token的内容
    symset set;
    int flag = 0;
	int savei_true = i_true;
	int savei_false = i_false;
	int notExist = 0;
//	int temp;
	int i=0;
	if (sym == SYM_NOT)
	{
		getsym(); //获取下一个token
		notExist = 1;
	}
    if (sym == SYM_ODD)//如果是odd运算符(一元)
    {
        getsym(); //获取下一个token
        expression(fsys); //对odd的表达式进行处理计算
        gen(OPR, 0, 6);//生成6号操作指令：奇偶判断运算
    }
    else if(sym == SYM_LPAREN)
    {
        getsym(); //获取一个token
        set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
        condition(set);//递归调用expression子程序分析一个子表达式
		if (!notExist)
		{
			if (!i_false)
			{
				gen(JMP, 0, 0);
				falselist[i_false++] = cx - 1;
			}
			while (i_true)
			{
				i_true--;
				code[truelist[i_true]].a = cx;
			}
		}
        destroyset(set);
        flag = 1;
        if (sym == SYM_RPAREN)//子表达式分析完后，应遇到右括号
        {
            getsym(); //如果的确遇到右括号，读取下一个token
        }
        else
        {
            error(22); // Missing ')'.
        }
    }
    else/* 如果不是odd 运算符(那就一定是二元逻辑运算符) */
    {
        set = uniteset(relset, fsys);
        expression(set);//对表达式左部进行处理计算
        destroyset(set);
        if (! inset(sym, relset))// 如果token不是逻辑运算符中的一个
        {
            gen(JNZ,0, 0);
            //error(20);
        }
        else
        {
            relop = sym; //记录下当前的逻辑运算符
            getsym();//获取下一个token
            expression(fsys); //对表达式右部进行处理计算
            switch (relop)//如果刚才的运算符是下面的一种
            {
                case SYM_EQU:
                //    jlist[i_jlist++] = cx;
                    gen(JE, 0, 0);
                    break;
                case SYM_NEQ:
                //  jlist[i_jlist++] = cx;
                    gen(JNE, 0, 0);
                    break;
                case SYM_LES:
                // jlist[i_jlist++] = cx;
                    gen(JL, 0, 0);
                    break;
                case SYM_GEQ:
                // jlist[i_jlist++] = cx;
                    gen(JGE, 0, 0);
                    break;
                case SYM_GTR:
                // jlist[i_jlist++] = cx;
                    gen(JG, 0, 0);
                    break;
                case SYM_LEQ:
                //jlist[i_jlist++] = cx;
                    gen(JLE, 0, 0);
                    break;
            } // switch
        } // else
    }// else
    // gen(OPR, 0,OPR_NOT);//生成13号操作指令：not判断运算
    //如果不是由NOT开头
	if (notExist)
	{
		while (i_true > savei_true)
		{
			i_true--;
			code[truelist[i_true]].a = cx;
		}
		while (i_false > savei_false)
		{
			i_false--;
			truelist[i_true++] = falselist[i_false];
		}
	}
    return flag;
}// condition_1

void condition_and(symset fsys)
{
	int flag;
	int hasAnd = 0;
    symset set;
    //int cx1_and;
    set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
    flag = condition_1(set);//每一个condition_and都应该由condition_1开始
	if (!flag && sym == SYM_AND)
	{
		code[cx - 1].f = 37 - code[cx - 1].f;
		falselist[i_false++] = cx - 1;
	}
    while (sym == SYM_AND)//一个condition_1后应当遇到AND
    {
		getsym();//获取下一个token
        flag = condition_1(set);
		if (!flag)
		{
			code[cx - 1].f = 37 - code[cx - 1].f;
			falselist[i_false++] = cx - 1;
		}
    	if(!i_false)
    	{
    		gen(JMP,0,0);
    		falselist[i_false++] = cx - 1;
    	}
        //code[cx1_and].a = cx;  code[cx_and[i_false]].a = cx;//L_falselist
        //gen(OPR, 0, OPR_AND);//生成AND指令14
        //i_false++;
    } // while
    destroyset(set);
}

void condition(symset fsys)
{
    symset set;
    //int cx1_or;
    set = uniteset(fsys, createset(SYM_OR, SYM_NULL));
    condition_and(set);
    while (sym == SYM_OR)//一个condition_and后应当遇到OR
    {
    	while(i_false)
    	{
    		--i_false;
    		code[falselist[i_false]].a = cx;
    	}
    	truelist[i_true++] = cx - 1;
        getsym();//获取下一个token
        condition_and(set);
        //code[cx1_or].a = cx;  code[cx_or[i_true]].a = cx;//L_truelist
        //gen(OPR, 0, OPR_OR);//生成OR指令15
    } // while
	truelist[i_true++] = cx - 1;
    destroyset(set);
}

void call(int i, symset fsys)
{
    mask* mk;
    //mask* mk1;
    mk = (mask*) &table[i];
    getsym();
	symset set1, set;

    int n = 0;
    //int j;

    if(sym == SYM_LPAREN)
    {
        do
        {
            getsym();
            set1 = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
			set = uniteset(set1, fsys);
            expression(set);
			destroyset(set1);
			destroyset(set);
            n++;
            //getsym();
        }
        while(sym == SYM_COMMA);

        if(sym == SYM_RPAREN)
            getsym();
        else
            error(22);
        gen(PAS, 0, n);
        gen(CAL, level - mk->level, mk->address);
    }
    else error(16);

}


int ifAppear = 0;
int loopStart;
//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx4,j, flag;
	int save_ifalse;
	int save_falseList[100];
	symset set1, set, set2, set3;
	int jumpList[100];
	int list = 0;
	int saveLoopStart;
	char flagid[5];

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk;
		if (!(i = position(id)))
		{
				getsym();
				if(sym == SYM_COLON)
				{
					set1 = createset(SYM_COLON,SYM_NULL);
            		fsys = uniteset(set1, fsys);
            		destroyset(set1);
					flag=0;
					for(j=0;j<labeltotal;j++)
					{
						if(!strcmp(labelchar[j],id))
						flag=1;
					}
					if(flag)
					error(31);
					else{
					strcpy(labelchar[labeltotal],id);
					if(labeltotal >= 10)
						error(33);
					else
					{
						labelcx[labeltotal] = cx;
						labeltotal++;	
					}
				}
				getsym();
				}
				else
				error(11);
				test(fsys, phi, 19);
				statement(fsys);
			// Undeclared identifier.
		}
		else if (table[i].kind == ID_CONSTANT)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		else if (table[i].kind == ID_PROCEDURE)
		{
            call(i, fsys);
			gen(POP, 0, 0);
		}
		else
		{
			getsym();
			mk = (mask*)&table[i];
			if(sym == SYM_DIMSTART)
            {
                addr(mk,fsys);
            }
			if (sym == SYM_BECOMES)
			{
				getsym();
				expression(fsys);
				gen(STO, level - mk->level, mk->address);
			}
			else if (sym == SYM_ADDBECOMES)
            {
				getsym();
				gen(LOD, level - mk->level, mk->address);
				expression(fsys);
				gen(OPR, 0, OPR_ADD);
				gen(STO, level - mk->level, mk->address);
            }
			else if (sym == SYM_MINBECOMES)
            {
				getsym();
				gen(LOD, level - mk->level, mk->address);
				expression(fsys);
				gen(OPR, 0, OPR_MIN);
				gen(STO, level - mk->level, mk->address);
            }
			else if (sym == SYM_MULBECOMES)
            {
				getsym();
				gen(LOD, level - mk->level, mk->address);
				expression(fsys);
				gen(OPR, 0, OPR_MUL);
				gen(STO, level - mk->level, mk->address);
            }
			else if (sym == SYM_DIVBECOMES)
            {
				getsym();
				gen(LOD, level - mk->level, mk->address);
				expression(fsys);
				gen(OPR, 0, OPR_DIV);
				gen(STO, level - mk->level, mk->address);
            }
			else if (sym == SYM_INC)
			{
				gen(LOD, level - mk->level, mk->address);
				gen(INC, level - mk->level, mk->address);
				gen(POP, 0, 1);
				getsym();
			}
			else if (sym == SYM_DEC)
			{
				gen(LOD, level - mk->level, mk->address);
				gen(DEC, level - mk->level, mk->address);
				gen(POP, 0, 1);
				getsym();
			}

		}
	}
	else if (sym == SYM_INC)
	{
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				if (table[i].kind == ID_VARIABLE)
				{
					mask *mk;
					mk = (mask *)&table[i];
					if(sym == SYM_DIMSTART)//array element , deal with address
                    {
                        addr(mk,fsys);
					}
					gen(INC, level - mk->level, mk->address);
					gen(LOD, level - mk->level, mk->address);
					gen(POP, 0, 1);
				}
				else
					error(12);
			}
		}
		else
			error(14);
		getsym();
	}
	else if (sym == SYM_DEC)
	{
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				if (table[i].kind == ID_VARIABLE)
				{
					mask *mk;
					mk = (mask *)&table[i];
					if(sym == SYM_DIMSTART)//array element , deal with address
                    {
                        addr(mk,fsys);
					}
					gen(DEC, level - mk->level, mk->address);
					gen(LOD, level - mk->level, mk->address);
					gen(POP, 0, 1);
				}
				else
					error(12);
			}
		}
		else
			error(14);
		getsym();
	}
	/*else if (sym == SYM_CALL)
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
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called.
			}
			getsym();
		}
	}*/
	else if (sym == SYM_IF)
	{ // if statement
		ifAppear = 1;
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		if (!i_false)
		{
			falselist[i_false++] = cx;
			gen(JMP, 0, 0);
		}
		while(i_true)
		{
			i_true--;
			code[truelist[i_true]].a = cx;
		}
		//destroyset(set);
		//destroyset(set1);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		//cx1 = cx;
		//gen(JPC, 0, 0);
		set2 = createset(SYM_ELSE, SYM_ELIF, SYM_NULL);
		set3 = uniteset(set2, fsys);
		save_ifalse = 0;
		while (save_ifalse < i_false)
		{
			save_falseList[save_ifalse] = falselist[save_ifalse];
			save_ifalse++;
		}
		i_false = 0;
		statement(set3);
		while (i_false < save_ifalse)
		{
			falselist[i_false] = save_falseList[i_false];
			i_false++;
		}
		while (sym == SYM_ELIF)
		{
		    jumpList[list++] = cx;
            gen(JMP, 0, 0);
            while(i_false)
            {
                i_false--;
                code[falselist[i_false]].a = cx;
            }
			getsym();
			condition(set);
			if (!i_false)
			{
				falselist[i_false++] = cx;
				gen(JMP, 0, 0);
			}
			while (i_true)
			{
				i_true--;
				code[truelist[i_true]].a = cx;
			}
			if (sym == SYM_THEN)
			{
				getsym();
			}
			else
			{
				error(16); // 'then' expected.
			}
			//cx1 = cx;
			//gen(JPC, 0, 0);
			save_ifalse = 0;
			while (save_ifalse < i_false)
			{
				save_falseList[save_ifalse] = falselist[save_ifalse];
				save_ifalse++;
			}
			i_false = 0;
			statement(set3);
			while (i_false < save_ifalse)
			{
				falselist[i_false] = save_falseList[i_false];
				i_false++;
			}
		}
		destroyset(set);
		destroyset(set1);
		if (sym == SYM_ELSE)
		{
            jumpList[list++] = cx;
            gen(JMP, 0, 0);
            while(i_false)
            {
                i_false--;
                code[falselist[i_false]].a = cx;
            }
			getsym();
			statement(fsys);
		}
        while(i_false)
        {
            i_false--;
            code[falselist[i_false]].a = cx;
        }
		while (list--)
			code[jumpList[list]].a = cx;
		destroyset(set3);
		destroyset(set2);
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
		loopLevel++;
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		loopStart = cx;
		condition(set);
		if (!i_false)
		{
			falselist[i_false++] = cx;
			gen(JMP, 0, 0);
		}
		while (i_true)
		{
			i_true--;
			code[truelist[i_true]].a = cx;
		}

		destroyset(set1);
		destroyset(set);
		//cx2 = cx;
		//gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		save_ifalse = 0;
		while (save_ifalse < i_false)
		{
			save_falseList[save_ifalse] = falselist[save_ifalse];
			save_ifalse++;
		}
		i_false = 0;
		saveLoopStart = loopStart;
		statement(fsys);
		loopStart = saveLoopStart;
		while (i_false < save_ifalse)
		{
			falselist[i_false] = save_falseList[i_false];
			i_false++;
		}
		gen(JMP, 0, cx1);
		while (i_false)
		{
			i_false--;
			code[falselist[i_false]].a = cx;
		}
		while (breakCount)
		{
			if (breakList[breakCount - 1].level == loopLevel)
			{
				breakCount--;
				code[breakList[breakCount].cx].a = cx;
			}
			else
				break;
		}
		loopLevel--;
		//code[cx2].a = cx;
	}
	else if (sym == SYM_EXIT)
	{
		gen(STP, 0, 0);
		getsym();
	}
	else if (sym == SYM_RETURN)
	{
		getsym();
		expression(fsys);
		gen(REV, 0, 0);
	}
	else if (sym == SYM_BREAK)
	{
		if (!loopLevel)
			error(28);
		else
		{
			getsym();
			breakList[breakCount].cx = cx;
			breakList[breakCount].level = loopLevel;
			breakCount++;
			gen(JMP, 0, 0);
		}

	}
	else if (sym == SYM_CONTINUE)
	{
		if (!loopLevel)
			error(28);
		else
		{
			getsym();
			gen(JMP, 0, loopStart);
		}
	}
	else if (sym == SYM_FOR)
	{
		int conditionExist = 0;
		loopLevel++;
		getsym();
		if (sym == SYM_LPAREN)
		{
			getsym();
			if (sym != SYM_SEMICOLON)
				statement(fsys);
			if (sym != SYM_SEMICOLON)
				error(10);
			getsym();
			cx1 = cx;
			loopStart = cx;
			if (sym != SYM_SEMICOLON)
			{
				condition(fsys);
				conditionExist = 1;
				if (!i_false)
				{
					falselist[i_false++] = cx;
					gen(JMP, 0, 0);
				}
			}
			else
				gen(JMP, 0, 0);
			if (sym != SYM_SEMICOLON)
				error(10);
			getsym();
			cx4 = cx;
			if (sym != SYM_RPAREN)
			{
				set1 = createset(SYM_RPAREN, SYM_NULL);
				set = uniteset(fsys, set1);
				loopStart = cx;
				statement(set);
				destroyset(set);
				destroyset(set1);
				gen(JMP, 0, cx1);
			}
			else
			{
				cx4 = cx1;
			}
			if (sym != SYM_RPAREN)
				error(22);
			if (!conditionExist)
			{
				if (cx1 == cx - 1)
					cx--;
				else
					code[cx1].a = cx;
			}
			getsym();
			while (i_true)
			{
				i_true--;
				code[truelist[i_true]].a = cx;
			}
			save_ifalse = 0;
			while (save_ifalse < i_false)
			{
				save_falseList[save_ifalse] = falselist[save_ifalse];
				save_ifalse++;
			}
			i_false = 0;
			saveLoopStart = loopStart;
			statement(fsys);
			loopStart = saveLoopStart;
			while (i_false < save_ifalse)
			{
				falselist[i_false] = save_falseList[i_false] ;
				i_false++;
			}
			gen(JMP, 0, cx4);
			while (i_false)
			{
				i_false--;
				code[falselist[i_false]].a = cx;
			}
			while (breakCount)
			{
				if (breakList[breakCount - 1].level == loopLevel)
				{
					breakCount--;
					code[breakList[breakCount].cx].a = cx;
				}
				else
					break;
			}
			//code[cx2].a = cx;
			loopLevel--;
		}
		else
		{
			error(26); //'(' expected;
		}
	}
	else if(sym==SYM_PRT)
    {
        getsym();
        if(sym ==SYM_LPAREN)
        {
            getsym();
        }
        else
            error(33);
        if(sym ==SYM_RPAREN)
        {
            getsym();
            gen(OPR, 0, OPR_WTL);
        }
        else
        {
            set1 = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
            set = uniteset(set1, fsys);
            expression(set);
            destroyset(set1);
            destroyset(set);
            gen(OPR, 0, OPR_PRT);
            while(sym ==SYM_COMMA)
            {
                getsym();
                set1 = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
                set = uniteset(set1, fsys);
                expression(set);
                destroyset(set1);
                destroyset(set);
                gen(OPR, 0, OPR_PRT);
            }
            if(sym==SYM_RPAREN)
            getsym();

        }
    }
    else if (sym == SYM_READ)
    {
        getsym();
        if (sym == SYM_LPAREN)
        {
            getsym();
        }
        else
        {
            error(33); //'(' expected
        }
        if (sym == SYM_IDENTIFIER)
        {
            if ((i = position(id)) == 0)
            {
                error(11); //Undeclared identifier
                getsym();
            }
            else
            {
                switch (table[i].kind)
                {
                    mask *mk;
                case ID_CONSTANT:
                    error(12); 
					getsym();//Illegal assignment
                    break;
                case ID_PROCEDURE:
                    error(12);
                    getsym();
                    break;
                case ID_VARIABLE:
                    mk = (mask*) &table[i];
                    getsym();
                    if(sym == SYM_DIMSTART)
                    addr(mk,fsys);
                    gen(OPR, 0, OPR_RED);
                    gen(STO, level - mk->level, mk->address);

					break;
                }
            }
        }
        else
        {
            error(19);
			getsym(); //incorrect symbol
        }

        while (sym == SYM_COMMA)
        {
            getsym();
            if (sym == SYM_IDENTIFIER)
            {
                if ((i = position(id)) == 0)
                {
                    error(11);
                    getsym();
                }
                else
                {
                    switch (table[i].kind)
                    {
                        mask *mk;
                    case ID_CONSTANT:
                        error(12);
                        getsym();
                        break;
                    case ID_PROCEDURE:
                        error(12);
                        getsym();
                        break;
                    case ID_VARIABLE:
                        mk = (mask*) &table[i];
                        getsym();
                        if(sym == SYM_DIMSTART)
                        addr(mk,fsys);
                        gen(OPR, 0, OPR_RED);
                        gen(STO, level - mk->level, mk->address);

                        break;

                    }
                }
            }
            else
            {
                error(19);
                getsym();
            }

        }

        if (sym == SYM_RPAREN)
        {
            getsym();
        }
        else
        {
            error(22); //"Missing ')'."
        }

    }
    else 	if ( sym == SYM_GOTO)
	{
		flag = 0;
		getsym();
		if(sym != SYM_IDENTIFIER)
		error(8);
		else
		{
			getsym();
			if(sym != SYM_SEMICOLON)
			error(5);
			else
			{
				strcpy(gotochar[gotototal],id);
				gotocx[gotototal]=cx;
				gotototal++;
				gen(JMP,0,0);
			}
		}
	}
	
	test(fsys, phi, 19);
} // statement

int n = 0; //参数的个数
char tem[MAXIDLEN + 1];
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	int save_dx;
	symset set1, set;

	dx = 3 + n;
	block_dx = dx;
	mk = (mask*) &table[tx];
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
				if(sym == SYM_DIMSTART)
				    arraydeclaration();
				else
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
				if(sym == SYM_DIMSTART)
				    arraydeclaration();
				else
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
		{ // procedure declarations ******************************************************
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				//enter(ID_PROCEDURE);
				strcpy(tem, id);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}
            level++;
            n = 0;
            save_dx = dx;
            dx = 3;
            if(sym == SYM_LPAREN)
            {
                do
                {
                    getsym();
                    if(sym == SYM_IDENTIFIER)
                    {
                        if(sym == SYM_DIMSTART)
						     arraydeclaration();
						else
						     vardeclaration();//处理参数
                        n ++;
                    }
                }
                while(sym == SYM_COMMA);

                if(sym == SYM_RPAREN)
                {
                    getsym();//参数处理结束
                }
                else
                {
                    error(19);
                    getsym();
                }
            }
            dx = save_dx;
            level --;
            strcpy(id, tem);
            enter(ID_PROCEDURE);
            level ++;

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
	int use;
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	int temp;      // the temporary variable of the return
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
		if (i.f == STP)
		{
			printf("The program exited\n");
			break;
		}
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
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
            case OPR_NOT:
                if(stack[top]!=0) stack[top]=0;
                else stack[top]=1;
                break;
            case OPR_AND:
                top--;
                if((stack[top]!=0)&&stack[top+1]!=0)
                    stack[top]=1;
                    else stack[top]=0;
                break;
            case OPR_OR:
                top--;
                if((stack[top]==0)&&stack[top+1]==0)
                    stack[top]=0;
                else stack[top]=1;
                break;
            case OPR_PRT:
                printf("%d ",stack[top]);
                top--;
                break;
            case OPR_WTL:
                printf("\n");
                break;
            case OPR_RED:
                scanf("%d",&stack[++top]);
                break;
            case OPR_RS:
                top--;
                stack[top] >>= stack[top+1];
                break;
            case OPR_LS:
                top--;
                stack[top] <<= stack[top+1];
                break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case INC:
			stack[base(stack, b, i.l) + i.a]++;
			break;
		case DEC:
			stack[base(stack, b, i.l) + i.a]--;
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
        case RDM:
            if(i.a==0)
            {
                stack[top]=rand();
                top++;
            }
            else
            {
                stack[top]=rand()%i.a;
                top++;
            }
		case POP:
			if (i.a)
				top--;
			else if (use)
			{
				top--;
				use = 0;
			}
			break;
        case PAS:
            for(k = i.a; k > 0; k--)
            {
                stack[top + 3] = stack[top];
                top--;
            }
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
        case JZ:
            if (stack[top]==0)
                pc=i.a;
            top--;
            break;
        case JG:
            if (stack[top-1] > stack[top])
                pc = i.a;
            top--;
            break;
        case JL:
            if (stack[top-1] < stack[top])
                    pc = i.a;
                top--;
            break;
        case JGE:
                if (stack[top-1] >= stack[top])
                    pc = i.a;
                top--;
            break;
        case JLE:
                if (stack[top-1] <= stack[top])
                    pc = i.a;
                top--;
            break;
        case JE:
                if (stack[top-1] == stack[top])
                    pc = i.a;
                top--;
            break;
        case JNE:
                if (stack[top-1] != stack[top])
                    pc = i.a;
                top--;
            break;
        case JNZ:
                if (stack[top]!=0)
                    pc=i.a;
                top--;
            break;

		case REV:
			temp = stack[top];
			top = b;
			pc = stack[top + 2];
			b = stack[top + 1];
			stack[top] = temp;
			use = 1;
			printf("%d\n", stack[top]);
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
int main ()
{
	FILE* hbin;
	char s[80];
	int i,j;
	symset set, set1, set2;
    srand((unsigned)time(NULL));
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
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE ,SYM_FOR, SYM_EXIT, SYM_RETURN, SYM_BREAK, SYM_CONTINUE, SYM_GOTO, SYM_PRT, SYM_IDENTIFIER, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_INC, SYM_DEC, SYM_RDM, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;
	for(i = 0;i <10;i++)
	{
		labelcx[i] = 0;
		gotocx[i] = 0;
	}

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	for(i=0;i<gotototal;i++)
	{
		for(j=0;j<labeltotal;j++)
		if(!strcmp(gotochar[i],labelchar[j]))
		code[gotocx[i]].a = labelcx[j];
		
	}
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
	return 0;
 } // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
