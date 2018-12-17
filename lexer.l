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
    if( DEBUG_TOKEN ){
        printf("number token %d %s\n", token, yytext);
    }
    return token;
}

// string constant
// inline int strToken( int token ) {
//     yylval.string_const = new std::string( yytext, yyleng );
//     if( DEBUG_TOKEN ){
//         printf("string token %d %s\n", token, yytext);
//     }
//     return token;
// }

// identifier
inline int identifierToken( int token ) {
    yylval.string = new std::string( yytext, yyleng );
    if( DEBUG_TOKEN ){
        printf("identifier token %d %s\n", token, yytext);
    }
    return token;
}

// integer
inline double intToken( int token ) {
   yylval.int_const = (float) atoi(yytext);
   sscanf( yytext, "%d", &yylval );
   if( DEBUG_TOKEN ) {
      printf("int token %d %s\n", token, yytext);
   }
   return token;
}

// float
inline double floatToken( int token ) {
   yylval.float_const = (float)atof(yytext);
   sscanf( yytext, "%lf", &yylval );
   if( DEBUG_TOKEN ){
      printf("float token %d %s\n", token, yytext);
   }
   return token;
}

%}

KEYWORD void|int|float|char|if|else|for|while|return|break

IDENTIFIER [_a-zA-Z][_a-zA-Z0-9]*

OPERATOR [.][.][.]|[<>][<>][=]|[-][-]|[+][+]|[|][|]|[#][#]|[&][&]|[+\-*\/<>=!%^|&][=]|[<][<]|[->][>]|[<>&=+\/\-*(){}\[\]\.,%~!?:|^;]

ASSIGNMENT_OPERATOR (([<>][<>]|[*\/%+\-&^|])[=]|[=])

FRACTIONALCONSTANT (([0-9]*\.[0-9]+)|([0-9]+\.))

EXPONENTPART ([eE][+-]?[0-9]+)

FLOATINGSUFFI X ([flFL])

INTEGERSUFFIX ([uU][lL]|[lL][uU]|[uUlL])

DECIMALCONSTANT ([1-9][0-9]*)

OCTALCONSTANT ([0][0-7]*)

HEXCONSTANT ([0][xX][0-9A-Fa-f]+)

CHARCONSTANT ('(([\\]['])|([^']))+')

STRINGLITERAL ["](([\\]["])|([^"]))*["]

WHITESPACE [ \t\r\n]+

PREPROC [#][ ][0-9]+[ ]{STRINGLITERAL}[ 0-9]*

ALL .

%%


 // types
void|char|int|float	  { return T_TYPE_SPEC; }

 // condition
if                    { return numToken(T_IF); }
else                  { return numToken(T_ELSE); }
for                   { return numToken(T_FOR); }
while                 { return numToken(T_WHILE); }
break                 { return numToken(T_BREAK); }
return                { return numToken(T_RETURN); }

 // io function
printf                { return numToken(T_PRINTF); }

 // comparison operators
=                     { return numToken(T_EQUAL); }
==                    { return numToken(T_CMP_EQ); }
!=                    { return numToken(T_CMP_NE); }
<                     { return numToken(T_CMP_LT); }
<=                    { return numToken(T_CMP_LE); }
>                     { return numToken(T_CMP_GT); }
>=                    { return numToken(T_CMP_GE); }

 // brackets
[(]                    { return numToken(T_LPAREN); }
[)]                    { return numToken(T_RPAREN); }
[{]                    { return numToken(T_LBRACE); }
[}]                    { return numToken(T_RBRACE); }
[\[]                   { return numToken(T_LBRACKET); }
[\]]                   { return numToken(T_RBRACKET); }

 // logical operations
[|][|]		           { return T_LOGIC_OR; }
[&][&]		           { return T_LOGIC_AND; }

 // math operators
[+]                     { return numToken(T_PLUS); }
[-]                     { return numToken(T_MINUS); }
[/]                     { return numToken(T_DIV); }
[*]                     { return numToken(T_MUL); }
[%]                     { return numToken(T_REM); }

 // terminators & delimiters
[;]                     { return numToken(T_SEMI); }
[,]                     { return numToken(T_COMMA); }

 // others
[?]                     { return numToken(T_QUESTION); }
[:]                     { return numToken(T_COLON); }
[.]                     { return numToken(T_DOT); }
[-][>]                  { return numToken(T_ARROW); }

 //identifier
{IDENTIFIER}            { return identifierToken(T_IDENTIFIER); }

 // immediate value
 //[\"][.]*[\"]            { return strToken(T_CONST_STRING); }
[0-9]+[\.][0-9]*        { return floatToken(T_FLOAT_CONST); }
[0-9]+                  { return floatToken(T_INT_CONST); }

 //other tokens 
[/][/]                  { singleLineComment = true; }
[\n]                    { ++lineNumber; }
{WHITESPACE}		    { ; }

.                       { fprintf(stderr, "Invalid token\n"); exit(1); }

%%

void yyerror(char const *s) {
     fprintf (stderr, "Parse error : %s\n", s);
     exit(1);
}

int yywrap() {
    return 1;
}
