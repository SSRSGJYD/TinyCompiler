#include <iostream>
#include "ast.h"
#include "codegen.h"

using namespace std;

NBlock* g_root;
extern int yyparse();

int main(int argc, char **argv)
{
	//词法和语法解析
	yyparse();
	g_root->print("");
	
	//生成中间代码
	CodeGenContext context;
	context.generateCode(*g_root);
	
	return 0;
}
