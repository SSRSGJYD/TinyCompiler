%{
#include <string>
#include <stdio.h>
#include "ast.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.str = new string(yytext)
#define TOKEN(t) ( yylval.int_const = t)
%}

KEYWORD void|int|float|char|if|else|for|while|return|break

IDENTIFIER [_a-zA-Z][_a-zA-Z0-9]*

OPERATOR [.][.][.]|[<>][<>][=]|[-][-]|[+][+]|[|][|]|[#][#]|[&][&]|[+\-*\/<>=!%^|&][=]|[<][<]|[->][>]|[<>&=+\/\-*(){}\[\]\.,%~!?:|^;]

ASSIGNMENT_OPERATOR (([<>][<>]|[*\/%+\-&^|])[=]|[=])

FRACTIONALCONSTANT (([0-9]*\.[0-9]+)|([0-9]+\.))

EXPONENTPART ([eE][+-]?[0-9]+)

FLOATINGSUFFIX ([flFL])

INTEGERSUFFIX ([uU][lL]|[lL][uU]|[uUlL])

DECIMALCONSTANT ([1-9][0-9]*)

OCTALCONSTANT ([0][0-7]*)

HEXCONSTANT ([0][xX][0-9A-Fa-f]+)

CHARCONSTANT ('(([\\]['])|([^']))+')

STRINGLITERAL ["](([\\]["])|([^"]))*["]

WHITESPACE [\t\r\n]+

PREPROC [#][ ][0-9]+[ ]{STRINGLITERAL}[0-9]*

ALL .

%%

"int"                   SAVE_TOKEN; puts("T_INT");  return T_INT;
"float"                 SAVE_TOKEN; puts("T_FLOAT"); return T_FLOAT;
"char"                  SAVE_TOKEN; puts("T_CHAR"); return T_CHAR;
"void"                  SAVE_TOKEN; puts("T_VOID"); return T_VOID;
"string"                SAVE_TOKEN; puts("T_STRING"); return T_STRING;

"if"                    puts("T_IF"); return TOKEN(T_IF);
"else"                  puts("T_ELSE"); return TOKEN(T_ELSE);
"for"                   puts("T_FOR"); return TOKEN(T_FOR);
"while"                 puts("T_WHILE"); return TOKEN(T_WHILE);
"return"                puts("T_RETURN"); return TOKEN(T_RETURN);
"extern"                puts("T_EXTERN"); return TOKEN(T_EXTERN);

[_a-zA-Z][_a-zA-Z0-9]*  SAVE_TOKEN; printf("T_IDENTIFIER : %s\n",yytext); return T_IDENTIFIER;

[0-9]+[\.][0-9]*        SAVE_TOKEN; printf("T_FLOAT_CONST : %s\n",yytext); return T_FLOAT_CONST;
[0-9]+                  SAVE_TOKEN; printf("T_INT_CONST : %s\n",yytext); return T_INT_CONST;
\'(\\.|[^"])\'     		yylval.str = new string(&(yytext[1])); printf("T_CHAR_CONST : %c\n",yytext[1]); return T_CHAR_CONST;
\"(\\.|[^"])*\"         SAVE_TOKEN; puts("T_LITERAL"); return T_LITERAL;
"=="					puts("T_CMP_EQ"); return TOKEN(T_CMP_EQ);
"!="                    puts("T_CMP_NE"); return TOKEN(T_CMP_NE);
"<="                    puts("T_CMP_LE"); return TOKEN(T_CMP_LE);
">="                    puts("T_CMP_GE"); return TOKEN(T_CMP_GE);
"<"                     puts("T_CMP_LT"); return TOKEN(T_CMP_LT);
">"                     puts("T_CMP_GT"); return TOKEN(T_CMP_GT);
"="						puts("T_EQUAL"); return TOKEN(T_EQUAL);

"("                     puts("T_LPAREN"); return TOKEN(T_LPAREN);
")"                     puts("T_RPAREN"); return TOKEN(T_RPAREN);
"{"                     puts("T_LBRACE"); return TOKEN(T_LBRACE);
"}"                     puts("T_RBRACE"); return TOKEN(T_RBRACE);
"["                     puts("T_LBRACKET"); return TOKEN(T_LBRACKET);
"]"                     puts("T_RBRACKET"); return TOKEN(T_RBRACKET);

"."                     puts("T_DOT"); return TOKEN(T_DOT);
"->"                    puts("T_ARROW"); return TOKEN(T_ARROW);

"!"                     puts("T_NOT"); return TOKEN(T_NOT);
"++"                    puts("T_INC"); return TOKEN(T_INC);
"--"                    puts("T_DEC"); return TOKEN(T_DEC);

"+"                     puts("T_ADD"); return TOKEN(T_ADD);
"-"                     puts("T_SUB"); return TOKEN(T_SUB);
"*"                     puts("T_MUL"); return TOKEN(T_MUL);
"/"                     puts("T_DIV"); return TOKEN(T_DIV);

"&"                     puts("T_AND"); return TOKEN(T_AND);
"|"                     puts("T_OR"); return TOKEN(T_OR);
"^"                     puts("T_XOR"); return TOKEN(T_XOR);

";"                     puts("T_SEMI"); return TOKEN(T_SEMI);
","                     puts("T_COMMA"); return TOKEN(T_COMMA);

"//"                	;
" "                	;
[\t\r\n]+               ;

.                       printf("Unknown token:%s\n", yytext); yyterminate();

%%

void yyerror(char const *s) 
{
	fprintf (stderr, "Parse error : %s , wrong token : %s\n", s,yytext);
	exit(1);
}

int yywrap()
{
	return 1;
}
