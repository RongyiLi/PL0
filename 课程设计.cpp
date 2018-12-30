#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
using namespace std;
#define keyword_num 15
char keyword[][20] = { "program","const","var","procedure","begin","end",
"if","then","else","while","do","call","read","write","odd" };
char id[100][20];
int id_ptr = 0;
char integer[100];
char text[10000];
int ptr = 0;
char ch;
char token[100];
int token_ptr = 0;
int row = 1;
int syn = 0;
bool error = false;
int follow[][5] = {{0},
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
void Prog();
void Block();
void Condecl();
void Const();
void Vardecl();
void Proc();
void Body();
void Statement();
void Lexp();
void Exp();
void Term();
void Factor();

////////////////////////////////////文件读取//////////////////////////////////////// 
bool Init()
{
	int i = 0;
	fstream in;
	char filename[20];
	//cout<<"请输入文件名字"<<endl;
	//cin>>filename;
	strcpy(filename, "1.txt");
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
	if (type != -1)
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
		cout<<"[Syntax Error] expected primary-expression before "<<token<<endl;
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
		syn = 0;
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
			syn = i + 1;
			return true;
		}
	}
	return false;
}

void InsertVariable()
{
	strcpy(id[id_ptr++], token);
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
		else if (!IsKeyword())
		{
			InsertVariable();
			syn = 16;
		}
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
			Error(0);
		else
			syn = 17;
	}
	else if (ch == '+' || ch == '-')
		syn = 18;
	else if (ch == '*' || ch == '/')
		syn = 19;
	else if (ch == '=' || ch == '<' || ch == '>')
	{
		token[token_ptr++] = ch;
		GetChar();
		token[token_ptr++] = ch;
		token[token_ptr] = '\0';

		if (strcmp(token, "<>") == 0)
			syn = 20;
		else if (strcmp(token, "<=") == 0)
			syn = 20;
		else if (strcmp(token, ">=") == 0)
			syn = 20;
		else
		{
			token[token_ptr - 1] = '\0';
			ptr--;
			syn = 20;
		}
	}
	else if (ch == ':')
	{
		GetChar();
		if (ch == '=')
			syn = 21;
		else
		{
			Error(3, "=");
			ptr--;
		}
	}
	else if (ch == '(')
		syn = 22;
	else if (ch == ')')
		syn = 23;
	else if (ch == ',')
		syn = 24;
	else if (ch == ';')
		syn = 25;
	else
		if (ch != '\0')
			Error();
}

void SkipFollow(int type)
{
	int i=0;
	do{
		for(i=0; follow[type][i]!=0; i++)
			if(follow[type][i]==syn)
				break;
		if(follow[type][i]!=0)
			break;
		Move();
	}while(syn!=0);
}
//------------------------------------------------------词法分析器------------------------------------------- 



//////////////////////////////////////语法分析//////////////////////////////////////////// 
//<prog> → program <id>；<block>
void Prog()
{
	Move();
	if (syn != 1)	//program
		Error(2, "program");
	else
		Move();

	if (syn == 16)
		Move();
	else
		Error(2,"<id>");
		
		if (syn != 25)	//;
			Error(2, ";");
		else
			Move();

		Block();
		
		if(syn!=0)
			Error(4);

}

//<block> → [<condecl>][<vardecl>][<proc>]<body>
void Block()
{
	Condecl();
	Vardecl();
	Proc();
	Body();
}

//<condecl> → const <const>{,<const>};
void Condecl()
{
	if (syn == 2)  // const
	{
		Move();
		Const();

		Move();
		while (syn == 24)  //,
		{
			Move();
			Const();
			Move();
		}

		if (syn == 25)	//;
			Move();
		else
			Error(2,";");
	}
	else
		SkipFollow(2);
}


//<const> → <id>:=<integer>
void Const()
{
	if (syn == 16)		//<id>
	{
		Move();
		if (syn == 21)		//:=
		{
			Move();
			if (syn != 17)		//<integer>
				Error(1);
		}
	}
	else
		Error();
}


//<vardecl> → var <id>{,<id>};
void Vardecl()
{
	if (syn == 3)	//var
	{
		Move();
		if (syn == 16)
		{
			Move();
			while (syn == 24)	//,
			{
				Move();
				if (syn != 16)	//<id>
				{
					Error(1);
					break;
				}
				Move();
			}
			if (syn == 25)	//;
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
void Proc()
{
	if (syn == 4)	//procedure
	{
		Move();
		if (syn == 16)	//<id>
			Move();
		else
			Error(2,"<id>");
			
			if (syn == 22)		//(
				Move();
			else 
				Error(2,"(");
				
				while (syn == 16)	//<id>
				{
					Move();
					if (syn != 24)	//,
						break;
					Move();
				}
				
				if (syn != 23)	//)
					Error(2,")");
				else
					Move();
					
				if (syn != 25)	//;
					Error(2,";");
				else
					Move();

				Block();
				while (syn == 25)	//;
				{
					Move();
					Proc();
				}
	}
	else
		SkipFollow(5);
}


//<body> → begin <statement>{;<statement>}end
void Body()
{
	if (syn == 5)	//begin
	{
		Move();
		Statement();
		//; || statement的first集
		//while (syn == 25||(syn==16||syn==7||syn==10||syn==12||syn==5||syn==13||syn==14))
		while(syn==25) //;
		{
			/*if(syn!=25)
				Error(2,";");*/
			Move();
			Statement();
		}
	
		if (syn == 6)	//end
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
void Statement()
{
	switch (syn)
	{
	case 16:	//<id>
		Move();
		if (syn == 21)	//:=
		{
			Move();
			Exp();
		}
		
		else
			Error(1);
		break;
	case 7:    //if
		Move();
		Lexp();
		//Move();
		if (syn == 8)	//then
			Move();
		else
			Error(2, "then");

		Statement();
		if (syn == 9)	//else
		{
			Move();
			Statement();
		}
		break;
	case 10:   //while
		Move();
		Lexp();
		//Move();
		if (syn == 11)	//do
			Move();
		else
			Error(2, "do");

		Statement();
		break;
	case 12:	//call
		Move();
		if (syn == 16)	//<id>
			Move();
		else
			Error(1);

		if (syn == 22)
			Move();
		else
			Error(1);
		//exp的first集
		if (syn==18||syn==16||syn==17)
		{
			Exp();
			while (syn == 24)		//,
			{
				Move();
				Exp();
			}
		}
		//else
		//	Error(2,"<exp>");

		if (syn == 23)
			Move();
		else
			Error(1);
		break;
	case 5:     //<body>
		Body();
		break;
	case 13:	//read
		Move();
		if (syn == 22)	//(
			Move();
		else
			Error(2,"(");

		if (syn == 16)	//<id>
			Move();
		else
			Error(1);

		while (syn == 24)		//,
		{
			Move();
			if (syn != 16)
				break;
			Move();
		}

		if (syn == 23)	//)
			Move();
		else
			Error(2,")");
		break;
	case 14:	//write
		Move();
		if (syn == 22)	//(
			Move();
		else
			Error(2,"(");

		Exp();
		while (syn == 24)		//,
		{
			Move();
			Exp();
		}

		if (syn == 23)	//)
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
void Lexp()
{
	if (syn == 15)	//odd
	{
		Move();
		Exp();
	}
	else
	{
		Exp();
		if (syn == 20)	//<lop>
			Move();
		else
			Error(2,"<lop>");
		Exp();
	}
}

//<exp> → [+|-]<term>{<aop><term>}
void Exp()
{
	if (syn == 18)	// + | -
		Move();
	Term();
	while (syn == 18) // + | -
	{
		Move();
		Term();
	}
}


//<term> → <factor>{<mop><factor>}
void Term()
{
	Factor();
	while (syn == 19)	// * | / 
	{
		Move();
		Factor();
	}
}

//<factor>→<id>|<integer>|(<exp>)
void Factor()
{
	if (syn == 16 || syn == 17)  // <id>||<integer>
		Move();
	else if (syn == 22)	//(
	{
		Exp();
		Move();
		if (syn == 23)		//)
			Move();
		else
			Error(2,")");
	}
	else
		Error(1);
}
//--------------------------------------------语法分析----------------------------------------- 

int main()
{
	Init();
	Prog();
	system("pause");

	return 0;
}
