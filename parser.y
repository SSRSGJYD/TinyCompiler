%code requires{

#include "ast.h"
extern ast_Top *g_root; // A way of getting the AST out

//! This is to fix problems when generating C++
// We are declaring the functions provided by Flex, so
// that Bison generated code can call them.
int yylex(void);
void yyerror(const char *);

}

// Represents the value associated with any kind of
// AST node.
%union{
    const Base *statement;
    int int_const;
    float float_const;
    std::string *string;
}
                        
%token T_TYPE_SPEC T_IDENTIFIER
%token T_SEMI T_COMMA T_LPAREN T_LBRACE T_RBRACE T_LBRACKET T_RBRACKET 
%token T_QUESTION T_COLON 
%token T_LOG_OR T_LOG_AND T_OR T_XOR T_AND 
%token T_MUL T_DIV T_REM  T_NOT T_DOT T_ARROW T_INCDEC T_ADDSUB_OP T_ASSIGN_OPER T_EQUAL
%token T_INT_CONST T_FLOAT_CONST
%token T_IF T_WHILE T_FOR T_RETURN
%nonassoc T_RPAREN
%nonassoc T_ELSE
			
                        
%type <statement> ExtDef ExtDeclaration
			
%type <statement> FuncDef ParameterList Parameter ParamDeclarator
			
%type <statement> DeclarationList Declaration DeclarationSpec DeclarationSpec_T InitDeclarator InitDeclaratorList Declarator
			
%type <statement> StatementList Statement CompoundStatement CompoundStatement_2 SelectionStatement ExpressionStatement JumpStatement IterationStatement
			
%type <statement> Expression AssignmentExpression ConditionalExpression LogicalOrExpression LogicalAndExpression InclusiveOrExpression ExclusiveOrExpression AndExpression EqualityExpression RelationalExpression ShiftExpression AdditiveExpression MultiplicativeExpression CastExpression UnaryExpression PostfixExpression PostfixExpression2 ArgumentExpressionList PrimaryExpression

%type <number> Constant T_INT_CONST T_FLOAT_CONST
			
%type <string> T_IDENTIFIER MultDivRemOP UnaryOperator ASSIGN_OPER T_ASSIGN_OPER T_EQUAL EQUALUALITY_OP REL_OP T_AND T_ADDSUB_OP  T_NOT T_MUL T_DIV T_REM
                        
%start ROOT
                        
%%

ROOT:
	        ExtDef { ; }
		;

// EXTERNAL DEFINITION

ExtDef:
		ExtDeclaration { g_root->push($1); }
        |       ExtDef ExtDeclaration { g_root->push($2); }
		;

ExtDeclaration:
		Declaration { $$ = $1; }
        |       FuncDef { $$ = $1; }
		;

// FUNCTION DEFINITION

FuncDef:
		DeclarationSpec T_IDENTIFIER T_LPAREN ParameterList T_RPAREN CompoundStatement { $$ = new Function(*$2, $4, $6); }
		;

ParameterList:
		%empty { $$ = new ParamList(); }
	| 	Parameter { $$ = new ParamList($1); }
	|       ParameterList T_COMMA Parameter { $$->push($3); }
		;

Parameter:
		DeclarationSpec ParamDeclarator { $$ = $2; }
		;

ParamDeclarator:
		T_IDENTIFIER { $$ = new Parameter(*$1);}
		;

// Declaration

DeclarationList:
		Declaration { $$ = new DeclarationList($1); }
	|	DeclarationList Declaration { $$->push($2); }
		;

Declaration:
		DeclarationSpec InitDeclaratorList T_SEMI { $$ = $2; }
		;

DeclarationSpec:
		DeclarationSpec_T { ; }
	|	DeclarationSpec_T DeclarationSpec { ; }
		;

DeclarationSpec_T:
		T_TYPE_SPEC { ; }
	|	T_TYPE_QUAL { ; }
	|	T_STRG_SPEC { ; }
		;

InitDeclaratorList:
		InitDeclarator { $$ = new VariableDeclaration($1); }
	|       InitDeclaratorList T_COMMA InitDeclarator { $$->push($3); }
		;

InitDeclarator:
		Declarator { ; }
	|	Declarator T_EQUAL AssignmentExpression { ; }
		;

Declarator:
		T_IDENTIFIER {$$ = new Variable(*$1); }
		;

// Statement

StatementList:
		Statement { $$ = new StatementList($1); }
	|	StatementList Statement { $$->push($2); }
		;

Statement:
		CompoundStatement { $$ = $1; }
	|	SelectionStatement { $$ = $1; }
	|	ExpressionStatement { $$ = $1; }
	|   JumpStatement { $$ = $1; }
	|	IterationStatement { $$ = $1; }
		;

CompoundStatement:
		T_LBRACE CompoundStatement_2 { $$ = $2; }
		;

CompoundStatement_2:
		T_RBRACE { $$ = new CompoundStatement; }
	|	DeclarationList T_RBRACE { $$ = new CompoundStatement($1); }
	|	DeclarationList StatementList T_RBRACE { $$ = new CompoundStatement($1, $2); }
	|	StatementList T_RBRACE { $$ = new CompoundStatement($1); }
		;

SelectionStatement:
		T_IF T_LPAREN Expression T_RPAREN Statement { $$ = new SelectionStatement($5); }
|	T_IF T_LPAREN Expression T_RPAREN Statement T_ELSE Statement { $$ = new SelectionStatement($5, $7); }
		;

ExpressionStatement:
		T_SEMI { $$ = new ExpressionStatement(); }
	|	Expression T_SEMI { $$ = $1; }
		;

JumpStatement:
		T_RETURN ExpressionStatement { $$ = $2; }
		;

IterationStatement:
		T_WHILE T_LPAREN Expression T_RPAREN Statement { $$ = $5; }
	|	T_DO Statement T_WHILE T_LPAREN Expression T_RPAREN T_SEMI { $$ = $2; }
	|	T_FOR T_LPAREN Expression T_SEMI Expression T_SEMI Expression T_RPAREN Statement { $$ = $9; }
		;

// Expressions

Expression:
		AssignmentExpression { $$ = $1; }
		;

AssignmentExpression:
		UnaryExpression T_EQUAL AssignmentExpression { $$ = $1; }
		;

LogicalOrExpression:
		LogicalAndExpression { $$ = $1; }
	|	LogicalOrExpression T_LOGIC_OR LogicalAndExpression { $$ = $3; }
		;

LogicalAndExpression:
		EqualityExpression { $$ = $1; }
	|	LogicalAndExpression T_LOGIC_AND InclusiveOrExpression { $$ = $3; }
		;

EqualityExpression:
	    RelationalExpression { $$ = $1; }
	|	EqualityExpression EQUALUALITY_OP RelationalExpression { $$ = $3; }
		;

EQUALUALITY_OP:
		T_CMP_EQ { $$ = $1; }
	|	T_CMP_NE { $$ = $1; }
		;

RelationalExpression:
		AdditiveExpression { $$ = $1; }
	|       RelationalExpression REL_OP AdditiveExpression { $$ = $3; }
		;

REL_OP:
		T_CMP_LT { $$ = $1; }
	|	T_CMP_LE { $$ = $1; }
	|	T_CMP_GT { $$ = $1; }
	|	T_CMP_GE { $$ = $1; }
		;

AdditiveExpression:
		MultiplicativeExpression { $$ = $1; }
	|	AdditiveExpression T_ADDSUB_OP MultiplicativeExpression { $$ = $3; }
		;

MultiplicativeExpression:
		CastExpression { $$ = $1; }
	|	MultiplicativeExpression MultDivRemOP CastExpression { $$ = $3; }
		;

MultDivRemOP:
		T_MUL { $$ = $1; }
	|	T_DIV { $$ = $1; }
	|	T_REM { $$ = $1; }
		;

CastExpression:
		UnaryExpression { $$ = $1; }
	|	T_LPAREN T_TYPE_SPEC T_RPAREN CastExpression { $$ = $4; }
		;

UnaryExpression:
		PostfixExpression { $$ = $1; }
	|	T_INCDEC UnaryExpression { $$ = $2; }
	|	UnaryOperator CastExpression { $$ = $2; }
		;

UnaryOperator:
		T_AND { $$ = $1; }
	|	T_ADDSUB_OP { $$ = $1; }
	|	T_MUL { $$ = $1; }
	|	T_NOT { $$ = $1; }
		;

PostfixExpression:
		PrimaryExpression { $$ = $1; }
	|	PostfixExpression T_LBRACKET Expression T_RBRACKET { $$ = $3; }
	|	PostfixExpression T_LPAREN PostfixExpression2 { $$ = $3; }
	|	PostfixExpression T_DOT T_IDENTIFIER { $$ = new Expression(); }
	|	PostfixExpression T_ARROW T_IDENTIFIER { $$ = new Expression(); }
	|	PostfixExpression T_INCDEC { $$ = new Expression(); }
		;

PostfixExpression2:
		T_RPAREN { $$ = new Expression(); }
	|	ArgumentExpressionList T_RPAREN { $$ = $1; }
		;

ArgumentExpressionList:
		AssignmentExpression { $$ = $1; }
	|	ArgumentExpressionList T_COMMA AssignmentExpression { $$ = $3; }
		;

PrimaryExpression:
		T_IDENTIFIER { $$ = new Expression(); }
	|       Constant { $$ = new Expression(); }
	|	T_LPAREN Expression T_RPAREN { $$ = $2; }
		;

Constant:
		T_INT_CONST { $$ = $1; }
	|	T_FLOAT_CONST { $$ = $1; }
		;

%%
ast_Top *g_root; // Definition of variable (to match declaration earlier)
ast_Top *parseAST() {
    g_root = new ast_Top;
    yyparse();
    return g_root;
}