#ifndef CODEGEN_H
#define CODEGEN_H

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

#include <stack>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <typeinfo>
#include "ast.h"
#include "parser.hpp"

using namespace llvm;
using namespace llvm::sys::fs;
using std::unique_ptr;
using std::map;
using std::to_string;
using std::error_code;
using legacy::PassManager;

//标识符表
typedef map<string, Value*> SymTable;

//代码段信息维护类
class CodeGenBlock
{
public:
	BasicBlock *block;//llvm::BasicBlock对象指针，代码段
	Value *returnValue;//llvm::Value对象指针，返回值
	map<string,Value*> locals;//局部变量，变量名与数值对应表
	map<string,shared_ptr<NIdentifier>> types;//局部变量，变量名与类型对应表
	std::map<string, uint64_t> arraySize; //数组的大小
};

//全局编译信息维护类
class CodeGenContext
{
private:
	vector<CodeGenBlock*> blockStack;//全局代码段向量

public:
	LLVMContext llvmContext;//线程上下文
	IRBuilder<> builder;//辅助类
	unique_ptr<Module> module;//模块，对应变量、函数和类型的集合
	SymTable globalVars;//全局变量
	
	CodeGenContext();
	//获取变量数据
	Value* getSymbolValue(string name) const; 
	//获取变量类型
	shared_ptr<NIdentifier> getSymbolType(string name) const;
	//获取数组大小
	uint64_t getArraySize(string name) const;
	//设置变量数据
	void setSymbolValue(string name, Value* value);
	//设置变量类型
	void setSymbolType(string name, shared_ptr<NIdentifier> value);
	//设置数组大小
	void setArraySize(string name, uint64_t value);
	//获取当前代码段
	BasicBlock* currentBlock() const;
	//压入代码段
	void pushBlock(BasicBlock * block);
	//弹出代码段
	void popBlock();
	//设置当前代码段返回值
	void setReturnValue(Value* value);
	//获取当前代码段返回值
	Value* getReturnValue();
  
	//生成代码
	void generateCode(NBlock&);
};


#endif
