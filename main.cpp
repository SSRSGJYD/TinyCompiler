#include <iostream>
#include "ast.h"

using namespace std;

extern NBlock* root;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    cout << root << endl;
    return 0;
}
