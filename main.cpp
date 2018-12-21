#include <iostream>
#include "ast.h"

using namespace std;

NBlock* g_root;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    g_root->print("");
    return 0;
}
