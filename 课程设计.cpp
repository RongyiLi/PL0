#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
using namespace std;
#define stacksize 1000 
#define keyword_num 15
char keyword[][20] = { "program","const","var","procedure","begin","end",
"if","then","else","while","do","call","read","write","odd" };

/*符号*/
#define symnum 33
enum symbol{
	nul, programsym, constsym, varsym, proceduresym,
	beginsym, endsym, ifsym, thensym, elsesym, 
	whilesym, dosym, callsym, readsym, writesym, 
	oddsym,	ident,	number,
	pls,	mins, times,	slash,	becomes,
	eql,	neq,   lss,   leq,	 gtr,	geq,
	lparen, rparen,	comma,	semicolon
};

enum symbol sym;

enum object{
	constant,
	variable,
	procedur
};

/*指令*/
#define functiomnum 10
enum  function{ 
	lit,	opr,	lod,
	sto,	cal,	inte,
	jmp,	jpc,	red,
	wrt
};

int cx=0;
#define cxmax 1000
struct instruction{
	enum function f;  /*虚拟机代码指令*/ 
	int l;			  /*引用层与声明层的层次差*/ 
	int a;			  /*根据f的不同而不同*/ 
};
struct instruction code[cxmax];

#define tablestruct_name_max 10
#define table_max 100

int table_n=0;
struct tablestruct{
	char name[tablestruct_name_max];
	enum object kind;
	int val;	//数值仅const使用 
	int level;	//所处层
	int adr;
	int size;
};

struct tablestruct table[table_max];


char text[10000];
int ptr = 0;
char ch;
char token[100];
int num;
int token_ptr = 0;
int row = 1;
bool error = false;
bool analyzer_error=false;
int error_num=0;
int follow[][5] = { {0},
					{25,0},
					{3,4,5,0},
					{24,25},
					{4,5},
					{25,5},
					{25,6,9},
					{25,6,9} };

bool Init();
void Move();
void SkipFollow(int type);
void Error(int type = -1, const char *str = '\0');
void Analyzer();
void Prog();
void Block(int lev,int &tx,int dxx,int txx);
void Condecl(int &tx, int lev, int &dx);
void Const(int &tx, int lev, int &dx);
void Vardecl(int &tx, int lev, int &dx);
void Proc(int &tx, int lev, int &dx);
void Body(int current_procedure_tx, int tx, int lev);
void Statement(int current_procedure_tx, int tx, int lev);
void Lexp(int current_procedure_tx, int lev);
void Exp(int current_procedure_tx, int lev);
void Term(int current_procedure_tx,int lev);
void Factor(int current_procedure_tx,int lev);
int position(int current_procedure_tx); 
int position_procedure(int tx);
void Enter(enum object k,int &tx,int lev,int &dx);
int gen(enum function x,int y,int z); 
int base(int l, int *s, int b);
void interpret();





////////////////////////////////////文件读取//////////////////////////////////////// 
bool Init()
{
	int i = 0;
	fstream in;
	char filename[20];
	cout<<"请输入执行的程序文件名字:  "; 
	cin>>filename;
	//strcpy(filename, "4.txt");
	in.open(filename, ios::in);
	if (in.fail())
	{
		cout << "error open" << endl;
		return false;
	}

	in.get(ch);
	while (!in.eof())
	{
		text[i++] = ch;
		in.get(ch);
	}
	text[i] = '\0';
	in.close();
	return true;
}
//-------------------------------------文件读取---------------------------------------- 


///////////////////////////////////错误处理//////////////////////////////////////////// 

void Error(int type, const char *str)
{
	error_num++;
	if (type<10)	//running error不输出 
		cout << "第" << row << "行      ";
	switch (type)
	{
	case 0:
		cout << "[Lexical Error] illegal character " << token << "" << endl;
		break;
	case 1:
		cout << "[Syntax  Error] expected initializer before " << token << endl;
		break;
	case 2:
		cout << "[Syntax  Error] expected " << str << " before " << token << endl;
		break;
	case 3:
		cout << "[Lexical  Error] missing " << str << " before " << token << endl;
		break;
	case 4:
		cout<<"[Syntax Error] redundant"<<endl;
		break;
	case 5:
		cout<<"[Semantic Error] Undefined "<<token<<endl;
		break;
	case 6:
		cout<<"[Semantic Error] Error type"<<endl;
		break;
	case 7:
		cout<<"[Semantic Error] redeclaration of "<<token<<endl;
		break;
	case 8:
		cout<<"[Syntax Error] unknown error"<<endl;
		break;
	case 9:
		cout<<"[Semantic Error] parameter mismatch"<<endl;
		break;
	case 10:
		cout<<"[Running Error] stack overflows"<<endl;
		break;
	case 11:
		cout<<"[Running Error] division error"<<endl;
		break;
	}
}
//-----------------------------------错误处理-------------------------------------------- 

///////////////////////////////////////词法分析器////////////////////////////////////// 
void GetChar()
{
	if (ch != '\0')
		ch = text[ptr++];
	else
		sym = nul;
}

void GetBC()
{
	while (ch == ' ' || ch == '\n' || ch == '\t')
	{
		if (ch == '\n')
			row++;
		GetChar();
	}
}


bool IsLetter()
{
	if (('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z'))
		return true;
	else
		return false;
}

bool IsDigit()
{
	if ('0' <= ch && ch <= '9')
		return true;
	else
		return false;
}

bool IsKeyword()
{
	for (int i = 0; i<keyword_num; i++)
	{
		if (strcmp(token, keyword[i]) == 0)
		{
			sym = (enum symbol)(i + 1);
			return true;
		}
	}
	return false;
}

void to_int()
{
	int n=strlen(token),i;
	num=0;
	for(i=0; i<n; i++)
		num=10*num+(token[i]-'0');
}

void Reset()
{
	token[0] = '\0';
	token_ptr = 0;
	error = false;
}

void Move()
{
	Reset();
	GetChar();
	GetBC();
	token[0] = ch;
	token[1] = '\0';
	if (IsLetter())
	{
		while (true)
		{
			if (IsLetter() || IsDigit())
			{
				token[token_ptr++] = ch;
				GetChar();
			}
			else if (NULL == strchr("+-*/,;<>:=()\n\t ", ch))
			{
				if (ch == '\n')
					row++;
				token[token_ptr++] = ch;
				GetChar();
				error = true;
			}
			else
				break;
		}
		token[token_ptr] = '\0';
		ptr--;
		if (error)
			Error(0);
		sym = ident;
		IsKeyword();
	}
	else if (IsDigit())
	{
		while (true)
		{
			if (IsDigit())
			{
				token[token_ptr++] = ch;
				GetChar();
			}
			else if (NULL == strchr("+-*/,;<>:=()\n\t ", ch))
			{
				if (ch == '\n')
					row++;
				token[token_ptr++] = ch;
				GetChar();
				error = true;
			}
			else
				break;
		}
		token[token_ptr] = '\0';
		ptr--;
		if (error)
		{
			Error(0);
			num=0;
	    }
		else
			to_int();
		sym = number;
	}
	else if (ch == '+' )
		sym = pls;
	else if (ch == '-' )
		sym = mins;
	else if (ch == '*' )
		sym = times;
	else if (ch == '/' )
		sym = slash;
	else if (ch == '=' || ch == '<' || ch == '>')
	{
		token[token_ptr++] = ch;
		GetChar();
		token[token_ptr++] = ch;
		token[token_ptr] = '\0';

		if (strcmp(token, "<>") == 0)
			sym = neq;
		else if (strcmp(token, "<=") == 0)
			sym = leq;
		else if (strcmp(token, ">=") == 0)
			sym = geq;
		else
		{
			token[token_ptr - 1] = '\0';
			ptr--;
			if (strcmp(token, "=") == 0)
			sym = eql;
			else if (strcmp(token, "<") == 0)
			sym = lss;
			else if (strcmp(token, ">") == 0)
			sym = gtr;
		}
	}
	else if (ch == ':')
	{
		GetChar();
		sym = becomes;
		if(ch != '=')
		{
			Error(3, "=");
			ptr--;
		}
	}
	else if (ch == '(')
		sym = lparen;
	else if (ch == ')')
		sym = rparen;
	else if (ch == ',')
		sym = comma;
	else if (ch == ';')
		sym = semicolon;
/*	else
		if (ch != '\0')
			Error();*/ 
}

void SkipFollow(int type)
{
	/*int i=0;
	do{
		for(i=0; follow[type][i]!=0; i++)
			if(follow[type][i]==sym)
				break;
		if(follow[type][i]!=0)
			break;
		Move();
	}while(sym!=0);*/
	int i=0;
	for(i=0; follow[type][i]!=0; i++)
			if(follow[type][i]==sym)
				return;
	Error(8);
	while (sym!=0)
	{
		for(i=0; follow[type][i]!=0; i++)
			if(follow[type][i]==sym)
				break;
		if(follow[type][i]!=0)
			break;
		Move();
	}
}

//------------------------------------------------------词法分析器------------------------------------------- 
void Analyzer()
{
	Prog();
	if (sym!=nul)
		Error(4);
}


//////////////////////////////////////语法语义分析和中间代码产生//////////////////////////////////////////// 
//<prog> → program <id>；<block>
void Prog()
{
	int tx=0;
	Move();
	if (sym != programsym)	//program
		Error(2, "program");
	else
		Move();

	if (sym == ident)
		Move();
	else
		Error(2,"<id>");
		
		if (sym != semicolon)	//;
			Error(2, ";");
		else
			Move();

		Block(0,tx,3,tx);

}

//<block> → [<condecl>][<vardecl>][<proc>]<body>
/*lev :层数 
  tx ：table表的下标指针,是以值参数形式使用的。
  dxx: 如果有形参，从dxx开始
  txx: procedur 在table中定义的位置
  */ 
void Block(int lev,int &tx,int dxx,int txx)
{
	int dx,current_procedure_tx;
	dx=dxx;
	table[txx].adr=cx;	//procedur中的adr存放JMP指令的位置 
	gen(jmp,0,0);
	
	Condecl(tx,lev,dx);
	Vardecl(tx,lev,dx);
	current_procedure_tx=tx;
	Proc(tx,lev,dx);
	code[table[txx].adr].a=cx;    //开始生成当前过程的代码
	gen(inte,0,dx);  //生成分配内存代码
	Body(current_procedure_tx,tx,lev);
	
	//table[tx_temp].adr=cx;
	
	//table[tx_temp].adr=cx;     //当前过程代码地址
	table[txx].size=dx;    //
	
	gen(opr,0,0);
}

//<condecl> → const <const>{,<const>};
void Condecl(int &tx, int lev, int &dx)
{
	if (sym == constsym)  // const
	{
		Move();
		Const(tx,lev,dx);

		Move();
		while (sym == comma)  //,
		{
			Move();
			Const(tx,lev,dx);
			Move();
		}

		if (sym == semicolon)	//;
			Move();
		else
			Error(2,";");
	}
	else
		SkipFollow(2);
}


//<const> → <id>:=<integer>
void Const(int &tx, int lev, int &dx)
{
	if (sym == ident)		//<id>
	{
		Enter(constant,tx,lev,dx);
		Move();
		if (sym == becomes)		//:=
		{
			Move();
			if (sym != number)		//<integer>
			{
				Error(1);
				tx--;
			}
			else
				table[tx].val=num;
				
		}
	}
	else
		Error();
}


//<vardecl> → var <id>{,<id>};
void Vardecl(int &tx, int lev, int &dx)
{
	if (sym == varsym)	//var
	{
		Move();
		if (sym == ident)
		{
			Enter(variable,tx,lev,dx);
			Move();
			while (sym == comma)	//,
			{
				Move();
				if (sym != ident)	//<id>
				{
					Error(1);
					break;
				}
				else
					Enter(variable,tx,lev,dx);
				Move();
			}
			if (sym == semicolon)	//;
				Move();
		}
		else
		{
			Error(1);
			SkipFollow(4);
		}
	}
	else
		SkipFollow(4);
}

//<proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
void Proc(int &tx, int lev, int &dx)
{
	int txx,dxx;
	if (sym == proceduresym)	//procedure
	{
		Move();
		if (sym == ident)	//<id>
		{
			Enter(procedur,tx,lev,dx);
	
			Move();
		}
		else
			Error(2,"<id>");
			
		txx=tx;	
			if (sym == lparen)		//(
				Move();
			else 
				Error(2,"(");
			dxx=3;	
				while (sym == ident)	//<id>
				{
					Enter(variable,tx,lev+1,dxx);
					Move();
					if (sym != comma)	//,
						break;
					Move();
				}
				table[txx].val=dxx-3;	//形参的个数 
				if (sym != rparen)	//)
					Error(2,")");
				else
					Move();
					
				if (sym != semicolon)	//;
					Error(2,";");
				else
					Move();

				Block(lev+1,tx,dxx,txx);
				while (sym == semicolon)	//;
				{
					Move();
					Proc(tx,lev,dx);
				}
	}
	else
		SkipFollow(5);
}


//<body> → begin <statement>{;<statement>}end
void Body(int current_procedure_tx, int tx,int lev)
{
	if (sym == beginsym)	//begin
	{
		Move();
		Statement(current_procedure_tx,tx,lev);
		//; || statement的first集
		//while (sym == 25||(sym==16||sym==7||sym==10||sym==12||sym==5||sym==13||sym==14))
		while(sym==semicolon) //;
		{
			/*if(sym!=25)
				Error(2,";");*/
			Move();
			Statement(current_procedure_tx,tx,lev);
		}
	
		if (sym == endsym)	//end
			Move();
		else
			Error(2, "end");
		}
	else
	{
		Error(2, "begin");
		SkipFollow(6);
	}
}


/*
<statement> → <id> := <exp>
|if <lexp> then <statement>[else <statement>]
|while <lexp> do <statement>
|call <id>（[<exp>{,<exp>}]）
|<body>
|read (<id>{，<id>})
|write (<exp>{,<exp>})
*/
void Statement(int current_procedure_tx,int tx,int lev)
{
	int i,cx1_temp,cx2_temp;
	switch (sym)
	{
	case ident:	//<id>
		i=position(current_procedure_tx);
		if(i==0)
			Error(5);
		else if (table[i].kind!=variable)
			Error(6);
		Move();
		if (sym == becomes)	//:=
		{
			Move();
			Exp(current_procedure_tx,lev);
			gen(sto,lev-table[i].level,table[i].adr);
		}
		else
			Error(1);
		break;
	case ifsym:    //if
		Move();
		Lexp(current_procedure_tx,lev);
		//Move();
		if (sym == thensym)	//then
			Move();
		else
			Error(2, "then");
		
		cx1_temp=cx;
		gen(jpc,0,0);
		Statement(current_procedure_tx,tx,lev);
		cx2_temp=cx;
		gen(jmp,0,0);
		code[cx1_temp].a=cx;
		if (sym == elsesym)	//else
		{
			Move();
			Statement(current_procedure_tx,tx,lev);
		}
		code[cx2_temp].a=cx;
		break;
	case whilesym:   //while
		Move();
		cx1_temp=cx;
		Lexp(current_procedure_tx,lev);
		cx2_temp=cx;
		gen(jpc,0,0);
		
		if (sym == dosym)	//do
			Move();
		else
			Error(2, "do");

		Statement(current_procedure_tx,tx,lev);
		gen(jmp,0,cx1_temp);
		code[cx2_temp].a=cx;
		break;
	case callsym:	//call
		Move();
		if (sym == ident)	//<id>
		{
			i=position_procedure(tx);
			if(i==0)
				Error(5);
			else if (table[i].kind!=procedur)
				Error(6);			
			Move();
		}
		else
			Error(1);
		
		if (sym == lparen)	//(
			Move();
		else
			Error(2,"(");
		//exp的first集
		cx1_temp=table[i].val; //参数个数 
		if (sym==pls||sym==mins||sym==ident||sym==number)
		{
			Exp(current_procedure_tx,lev);
			gen(opr,0,7);
			cx1_temp--; 
			while (sym == comma)		//,
			{
				Move();
				Exp(current_procedure_tx,lev);
				gen(opr,0,7);
				cx1_temp--;
			}
		}
		if	(cx1_temp!=0)
			Error(9);
		//else
		//	Error(2,"<exp>");
		gen(cal,lev-table[i].level,table[i].adr);
		if (sym == rparen)
			Move();
		else
			Error(2,")");
		break;
	case beginsym:     //<body>
		Body(current_procedure_tx,tx,lev);
		break;
	case readsym:	//read
		Move();
		if (sym == lparen)	//(
			Move();
		else
			Error(2,"(");

		if (sym == ident)	//<id>
		{
			i=position(current_procedure_tx);
			if(i==0)
				Error(5);
			else if (table[i].kind!=variable)
				Error(6);
			
			//gen(opr,0,16);
			//gen(sto,lev-table[i].level,table[i].adr);
			gen(red,lev-table[i].level,table[i].adr);
			
			Move();
		}
		else
			Error(1);

		while (sym == comma)		//,
		{
			Move();
			if (sym != ident)
				break;
			i=position(current_procedure_tx);
			if(i==0)
				Error(5);
			else if (table[i].kind!=variable)
				Error(6);
			gen(red,lev-table[i].level,table[i].adr);
			Move();
		}

		if (sym == rparen)	//)
			Move();
		else
			Error(2,")");
		break;
	case writesym:	//write
		Move();
		if (sym == lparen)	//(
			Move();
		else
			Error(2,"(");
	 
		Exp(current_procedure_tx,lev);
		gen(wrt,0,0);
		gen(opr,0,15);
		while (sym == comma)		//,
		{
			Move();
			Exp(current_procedure_tx,lev);
			gen(wrt,0,0);
			gen(opr,0,15);
		}

		if (sym == rparen)	//)
			//gen(opr,0,14);    //生成输出指令，输出栈顶的值
			//gen(opr,0,15);  //输出换行
			Move();
		else
			Error(2,")");
		break;
	default:
		SkipFollow(7);
		break;
	}
}

//<lexp> → <exp> <lop> <exp>|odd <exp>
void Lexp(int current_procedure_tx,int lev)
{
	enum symbol lop;
	if (sym == oddsym)	//odd
	{
		Move();
		Exp(current_procedure_tx,lev);
		gen(opr,0,6);
	}
	else
	{
		Exp(current_procedure_tx,lev);
		if (sym == eql||sym == neq||sym == lss||
			sym == leq||sym == gtr||sym == geq)	//<lop>
		{
			lop=sym;
			Move();
		}
		else
			Error(2,"<lop>");
		Exp(current_procedure_tx,lev);
		
		switch(lop)
		{
			case eql:
				gen(opr,0,8);
				break;
			case neq:
				gen(opr,0,9);
				break;
			case lss:
				gen(opr,0,10);
				break;
			case geq:
				gen(opr,0,11);
				break;
			case gtr:
				gen(opr,0,12);
				break;
			case leq:
				gen(opr,0,13);
				break;
		}
	}
}

//<exp> → [+|-]<term>{<aop><term>}
void Exp(int current_procedure_tx,int lev)
{
	enum symbol aop=pls;
	if (sym == pls||sym== mins)	// + | -
	{
		aop=sym;
		Move();
	}
	Term(current_procedure_tx, lev);
	if(aop==mins)
		gen(opr,0,1);
	while (sym == pls||sym== mins) // + | -
	{
		aop=sym;
		Move();
		Term(current_procedure_tx, lev);
		if (aop==pls)
			gen(opr,0,2);
		else if(aop==mins)
			gen(opr,0,3);
	}
}


//<term> → <factor>{<mop><factor>}
void Term(int current_procedure_tx,int lev)
{
	enum symbol mop;
	Factor(current_procedure_tx,lev);
	while (sym == times|| sym==slash)	// * | / 
	{
		mop=sym;
		Move();
		Factor(current_procedure_tx,lev);
		if(mop==times)
			gen(opr,0,4);
		else if(mop==slash)
			gen(opr,0,5);
	}
}

//<factor>→<id>|<integer>|(<exp>)
void Factor(int current_procedure_tx,int lev)
{
	int i;
	if (sym == ident)	//<id>
	{
		i=position(current_procedure_tx);
			if(i==0)
				Error(5);
			else if (table[i].kind==procedur)
				Error(6);
			
			if(table[i].kind==constant)
				gen(lit,0,table[i].val);
			else if(table[i].kind==variable)
				gen(lod,lev-table[i].level,table[i].adr);
		Move();
	}
	else if (sym == number)  // <integer>
	{
		Move();
		gen(lit,0,num); 
	}
	else if (sym == lparen)	//(
	{
		Exp(current_procedure_tx,lev);
		Move();
		if (sym == rparen)		//)
			Move();
		else
			Error(2,")");
	}
	else
		Error(1);
}
//--------------------------------------------语法语义分析和中间代码产生----------------------------------------- 


///////////////////////////////////////表格管理//////////////////////////////////////
int position(int current_procedure_tx) 
{ 
	int i=current_procedure_tx,current_level=table[i].level;
	while(i!=0)
	{
		if(current_level-1==table[i].level)
			current_level--;
		if(current_level==table[i].level)
		{
			//cout<<i<<" ";
			if(strcmp(token,table[i].name)==0)
				break;
		}		
		i--;
	}
	//cout<<endl;
	return i;
}

int position_procedure(int tx)
{
	int i=tx;
	//cout<<"tx:"<<tx<<endl;
	while(i!=0 && strcmp(token,table[i].name)!=0)
	{
		i--;
	}
	return i;
}

void Enter(enum object k,int &tx,int lev,int &dx)
{
	int i;
	//先判断是否存在 
	if(k==procedur)
	{
		i=position_procedure(tx);
		if(i!=0 && table[i].kind==procedur)
			Error(7);
	}
	else
	{
		i=position(tx);
		if(i!=0 && (table[i].level==lev) &&(table[i].kind==constant || table[i].kind==variable) )
			Error(7);
	}
	tx++;
	strcpy(table[tx].name,token);
	table[tx].kind = k;
	switch(k)
	{
		case constant:
			table[tx].level=lev;
			break;
		case variable:
			table[tx].level=lev;
			table[tx].adr=dx;
			dx++;
			break;
		case procedur:
			table[tx].level=lev;
			break;
	}
}
//--------------------------------------------表格管理----------------------------------------- 

///////////////////////////////////////中间代码//////////////////////////////////////
int gen(enum function x,int y,int z)
{
	if(cx>=cxmax)
	{
		cout<<"program too long"<<endl;    //程序过长
		return -1; 
	}
	code[cx].f=x;
	code[cx].l=y;
	code[cx].a=z;
	cx++;
	return 0;
}
//--------------------------------------------中间代码-----------------------------------------

///////////////////////////////////////执行运行//////////////////////////////////////
//l 为层差，s为过程数组，b为此次过程基址 
int base(int l, int *s, int b)
{ 
	int i=b;
	while(l>0)
	{
		i=s[i];		//直接外过程的数据段基址 
		l--;
	}
	return i;
}  

void interpret()
{
	int s[stacksize];
	int t,b,p,temp;	//栈顶寄存器（指针）t，基址寄存器（指针）b，程序地址寄存器p，
	struct instruction	i;		//指令寄存器i
	
	t=0;
	b=0;
	p=0;
	s[0]=0;
	s[1]=0;
	s[2]=0;
	temp=3;
	
	do{
		i = code[p];
		p++;
		
		if(t>=stacksize)
		{
			Error(10);
			return ;
		}
	switch (i.f)
	{
		case lit:
			s[t]=i.a;
			t++;
			break;
		case opr:
			switch(i.a)
			{
				case 0:
					t=b;
					p=s[b+2];	//RA 返回地址
					b=s[b+1];	//DL 动态链 前面的基址  
					break;
				case 1:
					s[t-1]=-s[t-1];
					break;
				case 2:
					t--;
					s[t-1]=s[t-1]+s[t];
					break;
				case 3:
					t--;
					s[t-1]=s[t-1]-s[t];
					break;
				case 4:
					t--;
					s[t-1]=s[t-1]*s[t];
					break;
				case 5:
					t--;
					if(s[t]==0)
					{
						Error(11);
						return ;
					}
					s[t-1]=s[t-1]/s[t];
					break;
				case 6:
					s[t-1]=s[t-1]%2;
					break;
				case 7:
					if (t+temp>=stacksize)
					{
						Error(10);
						return;
					}
					t--;
					s[t+temp]=s[t];
					temp++;
					break;
				case 8:
					t--;
					s[t-1]=(s[t-1]==s[t]);
					break;
				case 9:
					t--;
					s[t-1]=(s[t-1]!=s[t]);
					break;
				case 10:
					t--;
					s[t-1]=(s[t-1]<s[t]);
					break;
				case 11:
					t--;
					s[t-1]=(s[t-1]>=s[t]);
					break;
				case 12:
					t--;
					s[t-1]=(s[t-1]>s[t]);
					break;
				case 13:
					t--;
					s[t-1]=(s[t-1]<s[t]);
					break;
				case 14:
					t--;
					cout<<s[t]<<'\n';
					break;
				case 15:
					cout<<'\n';
					break;
				case 16:
					cin>>s[t];		
					t++;
					break;
			}
			break;
		case lod:
			s[t]=s[base(i.l,s,b)+i.a];
			t++;
			break;
		case sto:
			t--;
			s[base(i.l,s,b)+i.a]=s[t];
			break;
		case cal:
			if(t+2>=stacksize)
			{
				Error(10);
				return ; 
			} 
			s[t]=base(i.l,s,b);	//SL
			s[t+1]=b;		//DL
			s[t+2]=p;		//RA
			b=t;
			p=i.a;
			temp=3;
			break;
		case inte:
			t+=i.a;
			break;
		case jmp:
			p=i.a;
			break;
		case jpc:
			t--;
			if(s[t]==0)
				p=i.a;
			break;
		case red:
			cin>>s[t];
			s[base(i.l,s,b)+i.a]=s[t];
			break;
		case wrt:
			t--;
			cout<<s[t];
			break; 
	}
	}while(p!=0);
	 
}
//--------------------------------------------执行运行-----------------------------------------

  
int main()
{
	Init();
	Analyzer();
	
	cout<<"-error: "<<error_num<<endl<<endl;
		
	if (error_num==0)
	{
		//表格table输出 
		/*for (int i=0; i<table_max; i++)
		cout<<table[i].name<<"  "<<table[i].kind<<"  "<<table[i].level<<"  "<<table[i].adr<<
		"  "<<table[i].val<<"  "<<table[i].size<<endl;
		*/
		/*if(table[i].kind==constant)
		cout<<table[i].name<<"  "<<table[i].kind<<"  "<<table[i].level<<"  "<<table[i].val<<"  "<<table[i].size<<endl;
		else
		cout<<table[i].name<<"  "<<table[i].kind<<"  "<<table[i].level<<"  "<<table[i].adr<<"  "<<table[i].size<<endl;
	*/
		interpret();
		
		char f[5];
		for (int i=0; i<cx; i++)
		{
			switch(code[i].f)
			{
				case lit:
					strcpy(f,"lit");
					break;
				case opr:
					strcpy(f,"opr");
					break;
				case lod:
					strcpy(f,"lod");
					break;
				case sto:
					strcpy(f,"sto");
					break;
				case cal:
					strcpy(f,"cal");
					break;
				case inte:
					strcpy(f,"int");
					break;
				case jmp:
					strcpy(f,"jmp");
					break;
				case jpc:
					strcpy(f,"jpc");
					break;
				case red:
					strcpy(f,"red");
					break;
				case wrt:
					strcpy(f,"wrt");
					break;
			}
			cout<<"("<<i<<")  "<<f<<" "<<code[i].l<<"  "<<code[i].a<<endl;	
		}
		
	}
	return 0;
}
