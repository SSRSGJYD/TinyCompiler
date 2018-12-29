#include <iostream>
#include <string.h>
#include "ast.h"
#include "codegen.h"

using namespace std;

NBlock* g_root;
extern int yyparse();
extern FILE* yyin;

int main(int argc, char **argv)
{
	//词法和语法解析
	if(argc <= 1)
	{
		cout<<"输入命令格式为：./compiler [input]"<<endl<<"[input]：待编译的cpp文件"<<endl;
		return 0;
	}
	else if(!(yyin = fopen(argv[1], "r")))
	{
		cout<<argv[1]<<"文件读入错误"<<endl;
		return 0;
	}

	yyparse();
	g_root->print("");
	
	string filename = strtok(argv[1],".");
	
	//生成中间代码
	CodeGenContext context;
	context.generateCode(*g_root,filename+".ll");

	//生成.o文件
	context.generateObj(filename+".o");
	
	return 0;
}
