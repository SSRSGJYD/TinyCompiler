#include <iostream>
#include "ast.h"
#include "codegen.h"
#include "objgen.h"

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

	//生成.o文件
	ObjGen(context);
	
	return 0;
}
