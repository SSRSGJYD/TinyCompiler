%code requires{

#include "ast.h"
extern ASTVector *g_root; // A way of getting the AST out

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
%token T_LOGIC_OR T_LOGIC_AND T_OR T_XOR T_AND T_NOT
%token T_EQUAL T_CMP_EQ T_CMP_GE T_CMP_GT T_CMP_LE T_CMP_LT T_CMP_NE
%token T_MUL T_DIV T_REMAIN T_ADD T_SUB T_INC T_DEC
%token T_DOT T_ARROW
%token T_INT_CONST T_FLOAT_CONST
%token T_IF T_WHILE T_FOR T_RETURN
%nonassoc T_RPAREN
%nonassoc T_ELSE

 // definition and declaration block	
%type <statement> ExtDef ExtDeclaration

 // function and variable definition
%type <statement> FuncDef ParameterList Parameter ParamDeclarator

 // declarations		
%type <statement> DeclarationList Declaration DeclarationSpec DeclarationSpec_T InitDeclarator InitDeclaratorList Declarator

 // statements	
%type <statement> StatementList Statement CompoundStatement CompoundStatement_2 SelectionStatement ExpressionStatement JumpStatement IterationStatement

 // expressions
%type <statement> Expression AssignmentExpression LogicalOrExpression LogicalAndExpression InclusiveOrExpression ExclusiveOrExpression AndExpression EqualityExpression RelationalExpression AdditiveExpression MultiplicativeExpression UnaryExpression PostfixExpression PostfixExpression2 ArgumentExpressionList PrimaryExpression

 // constants
%type <int_const> T_INT_CONST 
%type <float_const> T_FLOAT_CONST

 // identifier in string format
%type <string> T_IDENTIFIER

 // start symbol                       
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
	|	EqualityExpression T_CMP_EQ RelationalExpression { $$ = $3; }
	|	EqualityExpression T_CMP_NE RelationalExpression { $$ = $3; }
		;

RelationalExpression:
		AdditiveExpression { $$ = $1; }
	|       RelationalExpression T_CMP_LT AdditiveExpression { $$ = $3; }
	|       RelationalExpression T_CMP_LE AdditiveExpression { $$ = $3; }
	|       RelationalExpression T_CMP_GT AdditiveExpression { $$ = $3; }
	|       RelationalExpression T_CMP_GE AdditiveExpression { $$ = $3; }
		;

AdditiveExpression:
		MultiplicativeExpression { $$ = $1; }
	|	AdditiveExpression T_ADD MultiplicativeExpression { $$ = $3; }
	|	AdditiveExpression T_SUB MultiplicativeExpression { $$ = $3; }
		;

MultiplicativeExpression:
		UnaryExpression { $$ = $1; }
	|	MultiplicativeExpression T_MUL UnaryExpression { $$ = $3; }
	|	MultiplicativeExpression T_DIV UnaryExpression { $$ = $3; }
	|	MultiplicativeExpression T_REMAIN UnaryExpression { $$ = $3; }
		;

UnaryExpression:
		PostfixExpression { $$ = $1; }
	|	T_INC UnaryExpression { $$ = $2; }
	|	T_DEC UnaryExpression { $$ = $2; }
	|	T_AND UnaryExpression { $$ = $2; }
	|	T_ADD UnaryExpression { $$ = $2; }
	|	T_SUB UnaryExpression { $$ = $2; }
	|	T_MUL UnaryExpression { $$ = $2; }
	|	T_NOT UnaryExpression { $$ = $2; }
		;

PostfixExpression:
		PrimaryExpression { $$ = $1; }
	|	PostfixExpression T_LBRACKET Expression T_RBRACKET { $$ = $3; }
	|	PostfixExpression T_LPAREN PostfixExpression2 { $$ = $3; }
	|	PostfixExpression T_DOT T_IDENTIFIER { $$ = new Expression(); }
	|	PostfixExpression T_ARROW T_IDENTIFIER { $$ = new Expression(); }
	|	PostfixExpression T_INC { $$ = new Expression(); }
	|	PostfixExpression T_DEC { $$ = new Expression(); }
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
		T_IDENTIFIER   { $$ = new Expression(); }
	|   T_INT_CONST    { $$ = new Expression(); }
	|   T_FLOAT_CONST  { $$ = new Expression(); }
	|	T_LPAREN Expression T_RPAREN { $$ = $2; }
		;

%%

ASTVector *g_root; // Definition of variable (to match declaration earlier)
ASTVector *parseAST() {
    g_root = new ASTVector;
    yyparse();
    return g_root;
}