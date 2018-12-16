%{
#include <string>
#include <stdio.h>
#include "ast.h"
#include "parser.tab.h"


// whether debuging lexer
extern bool DEBUG_TOKEN;
// number of current line, help with error report
int lineNumber = 1; 


// functions deal with lexer, store token and return info

// ordinary notations
inline int numToken( int token ) {
    yylval.token = token;
    if( DEBUG_TOKEN ){
        printf("number token %d %s\n", token, yytext);
    }
    return token;
}

// string constant
inline int strToken( int token ) {
    yylval.string_const = new std::string( yytext, yyleng );
    if( DEBUG_TOKEN ){
        printf("string token %d %s\n", token, yytext);
    }
    return token;
}

// identifier
inline int identifierToken( int token ) {
    yylval.identifier = new std::string( yytext, yyleng );
    if( DEBUG_TOKEN ){
        printf("identifier token %d %s\n", token, yytext);
    }
    return token;
}

// integer
inline double intToken( int token ) {
   yylval.int_const = (float) atoi(yytext);
   sscanf( yytext, "%d", &yylval.int_const );
   if( DEBUG_TOKEN ) {
      printf("int token %d %s\n", token, yytext);
   }
   return token;
}

// float
inline double floatToken( int token ) {
   yylval.float_const = (float)atof(yytext);
   sscanf( yytext, "%lf", &yylval.float_const );
   if( DEBUG_TOKEN ){
      printf("float token %d %s\n", token, yytext);
   }
   return token;
}

%}


%%

 // condition
"if"                    return numToken(T_IF);
"else"                  return numToken(T_ELSE);
"for"                   return numToken(T_FOR);
"while"                 return numToken(T_WHILE);
"return"                return numToken(T_RETURN);

 // io function
"printf"                return numToken(T_PRINTF);

 // immediate value
\".*\"                  return strToken(T_STR);
[0-9]+\.[0-9]*          return floatToken(T_NUM_DOUBLE);
[0-9]+                  return floatToken(T_NUM_INTEGER);

 //identifier
[_a-zA-Z][_a-zA-Z0-9]*  return identifierToken(T_IDENTIFIER);

 // comparison operators
"="                     return numToken(T_EQUAL);
"=="                    return numToken(T_CMP_EQ);
"!="                    return numToken(T_CMP_NE);
"<"                     return numToken(T_CMP_LT);
"<="                    return numToken(T_CMP_LE);
">"                     return numToken(T_CMP_GT);
">="                    return numToken(T_CMP_GE);

 // brackets
"("                     return numToken(T_LPAREN);
")"                     return numToken(T_RPAREN);
"{"                     return numToken(T_LBRACE);
"}"                     return numToken(T_RBRACE);

 // math operators
"+"                     return numToken(T_PLUS);
"-"                     return numToken(T_MINUS);
"/"                     return numToken(T_DIV);
"*"                     return numToken(T_MUL);

 // terminators & delimiters
";"                     return numToken(T_SEMI);
","                     return numToken(T_COMMA);

 // whitespace
[ \r\t]+ { }

"//"                    { singleLineComment = true; }
\n                      { ++lineNumber; }
.                       yyterminate();

%%

int yywrap()
{
    return 1;
}
