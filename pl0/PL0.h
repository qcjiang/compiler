#include <stdio.h>

#define NRW        25  // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       15     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols

#define STACKSIZE  1000   // maximum storage
#define GOTONUM 100//最多goto数量

enum symtype              //记号类型
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
	SYM_ELSE,
	SYM_ELIF,
	SYM_EXIT,
	SYM_RETURN,
	SYM_FOR,
	SYM_AND,
	SYM_OR,
	SYM_NOT,
	SYM_OP_AND,
	SYM_OP_OR,
	SYM_LBRACKET,
	SYM_RBRACKET,
	SYM_DEC,
	SYM_INC,
	SYM_BREAK,
	SYM_CONTINUE,
	SYM_PRT,
	SYM_RDM,
	SYM_DIMSTART,
	SYM_DIMEND,
	SYM_COLON,
	SYM_GOTO,
	SYM_READ,
	SYM_LS,
	SYM_RS,
	SYM_ADDBECOMES,
	SYM_MINBECOMES,
	SYM_MULBECOMES,
	SYM_DIVBECOMES
};

enum idtype      //标识符类型
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE,ID_GO
};

enum opcode     //操作码
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC,
	STP, REV, INC, DEC, NOT, OR, AND,JZ, JG,//16
	JL,JE, JNE, JGE, JLE,//21
    JNZ,
	 PAS , POP,RDM
};

enum oprcode    //汇编指令的运算操作码
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT, OPR_AND, OPR_OR,
	OPR_WTL, OPR_PRT, OPR_RED, OPR_LS,
	OPR_RS
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
	/*  0 */    "",
	/*  1 */    "Found ':=' when expecting '='.",
	/*  2 */    "There must be a number to follow '='.",
	/*  3 */    "There must be an '=' to follow the identifier.",
	/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */    "Missing ',' or ';'.",
	/*  6 */    "Incorrect procedure name.",
	/*  7 */    "Statement expected.",
	/*  8 */    "Follow the statement is an incorrect symbol.",
	/*  9 */    "'.' expected.",
	/* 10 */    "';' expected.",
	/* 11 */    "Undeclared identifier.",
	/* 12 */    "Illegal assignment.",
	/* 13 */    "':=' expected.",
	/* 14 */    "There must be an identifier to follow.",
	/* 15 */    "A constant or variable can not be called.",
	/* 16 */    "'then' expected.",
	/* 17 */    "';' or 'end' expected.",
	/* 18 */    "'do' expected.",
	/* 19 */    "Incorrect symbol.",
	/* 20 */    "Relative operators expected.",
	/* 21 */    "Procedure identifier can not be in an expression.",
	/* 22 */    "Missing ')'.",
	/* 23 */    "The symbol can not be followed by a factor.",
	/* 24 */    "The symbol can not be as the beginning of an expression.",
	/* 25 */    "The number is too great.",
	/* 26 */    "'(' expected",
	/* 27 */    "'if' doesn't appear before",
	/* 28 */    "'break' or 'continue' should be in loop" ,
/* 29 */    "']'expected",
/* 30 */    "constant expected",
/* 31 */    "same gotoname declared",
/* 32 */    "There are too many levels.",
/*33*/ "too many goto",
/*34*/
};//表示各个错误类型的字符串

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;        //error次数
int  cx;
int truelist[100];
int falselist[100];
int i_true;
int i_false;        // index of current instruction to be generated.
int loopLevel;
int breakCount = 0;
int  level = 0;
int  tx = 0;

char line[80];

int labeltotal = 0;
int gotototal = 0;
char labelchar[10][11];
int labelcx[10];
char gotochar[10][11];
int gotocx[10];


instruction code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while","else","elif","exit","return","for", "not",  "and", "or", "break", "continue", "print"
	,"random", "read", "goto"
	//25
};   //保留字

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,SYM_IF, SYM_ODD, SYM_PROCEDURE,
	SYM_THEN, SYM_VAR, SYM_WHILE,SYM_ELSE,SYM_ELIF,SYM_EXIT,SYM_RETURN,SYM_FOR,
	SYM_NOT,
	SYM_AND,
	SYM_OR, SYM_BREAK, SYM_CONTINUE, SYM_PRT,SYM_RDM, SYM_READ, SYM_GOTO
};   //读取的记号如果为保留字，在word数组中匹配得到索引，再用索引读出wsym对应的枚举值


int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON, SYM_LBRACKET, SYM_RBRACKET,
	SYM_NOT,
	SYM_AND,
	SYM_OR

};//读取的记号如果是运算符，在csym数组中匹配得到索引，再用索引读出csym对应的枚举值

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';','[',']', '!', '&', '|'
};//运算符

#define MAXINS   26
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "STP", "REV",
	"INC", "DEC", "NOT", "AND", "OR", "JZ", "JG", "JL", "JE", "JNE",
	 "JGE", "JLE", "JNZ" ,"PAS" , "POP","RDM"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
} mask;

typedef struct
{
	int cx;
	int level;
}breakStru;

breakStru breakList[100];


FILE* infile;

// EOF PL0.h
