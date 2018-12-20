%code requires
{

	#include "ast.h"
	NBlock *root;//AST根节点
	extern int yylex();
	void yyerror(const char *);
	int yydebug=1;

}

//AST节点类型
%union
{
	NBlock* block;//代码段
	NExpression* expression;//表达式
	NStatement* statement;//语句
	NIdentifier* identifier;//变量与关键字标识符
	vector<shared_ptr<NVariableDeclaration>>* var_vector;//变量列表
	std::vector<shared_ptr<NExpression>>* expression_vector;//表达式列表
	int int_const;//整型常量
	float float_const;//浮点型常量
	string *str;//字符串
}

/*-------------token---------------*/
//变量类型与变量名：int float char void 变量名
%token <str> T_INT T_FLOAT T_CHAR T_VOID T_IDENTIFIER
//int常量与float常量
%token <str> T_INT_CONST T_FLOAT_CONST
//赋值符号与比较符号，依次为：= == >= > <= < !=
%token <int_const> T_EQUAL T_CMP_EQ T_CMP_GE T_CMP_GT T_CMP_LE T_CMP_LT T_CMP_NE
//各种标点符号，依次为：; , ( ) { } [ ] . ->
%token <int_const> T_SEMI T_COMMA T_LPAREN T_RPAREN T_LBRACE T_RBRACE T_LBRACKET T_RBRACKET T_DOT T_ARROW
//各种运算符号，依次为：|| && | ^ & ! * / % + - ++ -- -
%token <int_const> T_LOGIC_OR T_LOGIC_AND T_OR T_XOR T_AND T_NOT T_MUL T_DIV T_MOD T_ADD T_SUB T_INC T_DEC
//判断、循环、返回语句，依次为：if else while for return
%token <int_const> T_IF T_ELSE T_WHILE T_FOR T_RETURN T_EXTERN

/*-------------type---------------*/
//变量标识符，依次为：变量 类型名
%type <identifier> Identifier Typename
//表达式，依次为：一般表达式 赋值表达式 数字表达式
%type <expression> Expression AssignmentExpression NumberExpression
//运算式，依次为：一元运算式 二元运算式
%type <expression> UnaryExpression BinaryExpression
//函数声明的参数列表
%type <var_vector> FuncParameter
//函数调用的参数列表
%type <expression_vector> CallParameter
//代码段
%type <block> Root Statements Block
//语句，依次为：一般语句 条件语句 循环语句 变量定义语句 函数定义语句
%type <statement> Statement SelectionStatement IterationStatement VarDeclaration FuncDeclaration

//定义优先级
%left T_EQUAL // =
%left T_LOGIC_OR // ||
%left T_LOGIC_AND // &&
%left T_OR // |
%left T_XOR // ^
%left T_AND // &
%left T_CMP_EQ T_CMP_NE // == !=
%left T_CMP_GE T_CMP_GT T_CMP_LE T_CMP_LT // >= > <= <
%left T_ADD T_SUB // + -
%left T_MUL T_DIV T_MOD// * / %
%left T_INC T_DEC T_NOT// ++ -- !
%left T_LPAREN T_RPAREN T_DOT T_ARROW // () . ->


//开始                      
%start Root
                        
%%

/*-------------代码段---------------*/
Root:Statements { root = $1; };

Statements : Statement { $$ = new NBlock(); $$->statements->push_back(shared_ptr<NStatement>($1)); }
		| Statements Statement { $1->statements->push_back(shared_ptr<NStatement>($2)); }
		;
		
Block : T_LBRACE Statements T_RBRACE { $$ = $2; }
		| T_LBRACE T_RBRACE { $$ = new NBlock(); }
		;

/*-------------变量标识符---------------*/
Identifier : T_IDENTIFIER { $$ = new NIdentifier(*$1); delete $1; };
		
Typename : T_VOID { $$ = new NIdentifier(*$1); $$->isType = true;  delete $1; }
		| T_INT { $$ = new NIdentifier(*$1); $$->isType = true;  delete $1; }
		| T_FLOAT { $$ = new NIdentifier(*$1); $$->isType = true;  delete $1; }
		| T_CHAR { $$ = new NIdentifier(*$1); $$->isType = true;  delete $1; }
		;
   
/*-------------表达式---------------*/
Expression : AssignmentExpression { $$ = $1; }
		 | Identifier T_LPAREN CallParameter T_RPAREN { $$ = new NFunctionCall(shared_ptr<NIdentifier>($1), shared_ptr<ExpressionList>($3)); }
		 | Identifier { $$ = $1; }
		 | NumberExpression
		 | BinaryExpression
		 | UnaryExpression
		 | T_LPAREN Expression T_RPAREN { $$ = $2; }
		 ;

AssignmentExpression : Identifier T_EQUAL Expression { $$ = new NAssignment(shared_ptr<NIdentifier>($1), shared_ptr<NExpression>($3)); };

NumberExpression : T_INT { $$ = new NConstant<int>(atol($1->c_str())); }
		| T_FLOAT { $$ = new NConstant<float>(atof($1->c_str())); }
		;

/*-------------运算式---------------*/
BinaryExpression : Expression T_CMP_EQ Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_CMP_GE Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_CMP_GT Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); } 
		| Expression T_CMP_LE Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); } 
		| Expression T_CMP_LT Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_CMP_NE Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_LOGIC_OR Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_LOGIC_AND Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_OR Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_XOR Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_AND Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_MUL Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_DIV Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_MOD Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_ADD Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		| Expression T_SUB Expression { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		;	
						
UnaryExpression : T_INC Expression { $$ = new NUnaryOperator($1, shared_ptr<NExpression>($2)); }
	| Expression T_INC { $$ = new NUnaryOperator($2, shared_ptr<NExpression>($1)); }
	| T_DEC Expression { $$ = new NUnaryOperator($1, shared_ptr<NExpression>($2)); }
	| Expression T_DEC { $$ = new NUnaryOperator($2, shared_ptr<NExpression>($1)); }
	| T_NOT Expression { $$ = new NUnaryOperator($1, shared_ptr<NExpression>($2)); }
	;


/*-------------函数声明参数列表---------------*/	
FuncParameter : /* 空 */ { $$ = new VariableList(); }
		| VarDeclaration { $$ = new VariableList(); $$->push_back(shared_ptr<NVariableDeclaration>($1)); }
		| FuncParameter T_COMMA VarDeclaration { $1->push_back(shared_ptr<NVariableDeclaration>($3)); }
		;

/*-------------函数调用参数列表---------------*/	
CallParameter : /* 空 */ { $$ = new ExpressionList(); }
		| Expression { $$ = new ExpressionList(); $$->push_back(shared_ptr<NExpression>($1)); }
		| CallParameter T_COMMA Expression { $1->push_back(shared_ptr<NExpression>($3)); }
		;
		
/*-------------语句---------------*/	 
Statement : VarDeclaration
		| FuncDeclaration
		| SelectionStatement
		| IterationStatement
		| T_SEMI { $$ = new NExpressionStatement(); }
		| Expression T_SEMI { $$ = new NExpressionStatement(shared_ptr<NExpression>($1)); }
		| T_RETURN Expression T_SEMI { $$ = new NReturnStatement(shared_ptr<NExpression>($2)); }
		;

VarDeclaration : Typename Identifier T_SEMI { $$ = new NVariableDeclaration(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($2), nullptr); }
		| Typename Identifier T_EQUAL Expression T_SEMI { $$ = new NVariableDeclaration(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($2), shared_ptr<NExpression>($4)); }
		;

FuncDeclaration : Typename Identifier T_LPAREN FuncParameter T_RPAREN Block { $$ = new NFunctionDeclaration(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($2), shared_ptr<VariableList>($4), shared_ptr<NBlock>($6)); }
		| T_EXTERN Typename Identifier T_LPAREN FuncParameter T_RPAREN T_SEMI { $$ = new NFunctionDeclaration(shared_ptr<NIdentifier>($2), shared_ptr<NIdentifier>($3), shared_ptr<VariableList>($5), nullptr, true); }
		;
		
SelectionStatement : T_IF Expression Block { $$ = new NIfStatement(shared_ptr<NExpression>($2), shared_ptr<NBlock>($3)); }
		| T_IF Expression Block T_ELSE Block { $$ = new NIfStatement(shared_ptr<NExpression>($2), shared_ptr<NBlock>($3), shared_ptr<NBlock>($5)); }
		| T_IF Expression Block T_ELSE SelectionStatement 
		{
			auto blk = new NBlock();
			blk->statements->push_back(shared_ptr<NStatement>($5));
			$$ = new NIfStatement(shared_ptr<NExpression>($2), shared_ptr<NBlock>($3), shared_ptr<NBlock>(blk)); 
		}
		;

IterationStatement : T_FOR T_LPAREN Expression T_SEMI Expression T_SEMI Expression T_RPAREN Block { $$ = new NForStatement(shared_ptr<NBlock>($9), shared_ptr<NExpression>($3), shared_ptr<NExpression>($5), shared_ptr<NExpression>($7)); }
		| T_WHILE T_LPAREN Expression T_RPAREN Block { $$ = new NForStatement(shared_ptr<NBlock>($5), nullptr, shared_ptr<NExpression>($3), nullptr); }
		;

%%
