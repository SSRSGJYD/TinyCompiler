#ifndef AST_H
#define AST_H

#include "llvm/IR/Value.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cassert>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;

class CodeGenContext;
class NBlock;
class NStatement;
class NExpression;
class NVariableDeclaration;

//表达式列表别名
typedef vector<shared_ptr<NStatement>> StatementList;
//语句列表别名
typedef vector<shared_ptr<NExpression>> ExpressionList;
//变量列表别名
typedef vector<shared_ptr<NVariableDeclaration>> VariableList;

//所有节点的基类
class Node
{
protected:
	const char m_DELIM = ':';
	const char* m_PREFIX = "--";
public:
	Node(){}
	virtual ~Node() {}
	//获取节点类型名
	virtual string getTypeName() const = 0;
	//打印节点
	virtual void print(string prefix) const{}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context) { return (llvm::Value *)0; }
};

//表达式的基类
class NExpression : public Node
{
public:
	NExpression(){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NExpression";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		cout << prefix << getTypeName() << endl;
	}
};

//语句的基类
class NStatement : public Node
{
public:
	NStatement(){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NStatement";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		cout << prefix << getTypeName() << endl;
	}
};

/*-------------表达式的子类---------------*/
//常量类（有double、int、char）
template<typename T>
class NConstant : public NExpression
{
public:
	T value;
	NConstant(){}
	NConstant(T value):value(value){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NConstant";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		cout << prefix << getTypeName() << this->m_DELIM << value << endl;
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

//变量类（有一般变量、类型名）
class NIdentifier : public NExpression
{
public:
	string name;//变量名称
	bool isType = false;//是否为类型名
	bool isArray = false;
	uint64_t arraySize = 0;
	
	NIdentifier(){}
	NIdentifier(const std::string &name):name(name) {}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NIdentifier";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		if (isArray) {
			cout << prefix << getTypeName() << this->m_DELIM << name << '[' << this->arraySize << ']' << endl;
		} else {
			cout << prefix << getTypeName() << this->m_DELIM << name << endl;
		}
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

//函数调用类
class NFunctionCall : public NExpression
{
public:
	const shared_ptr<NIdentifier> id;//函数对应变量
	shared_ptr<ExpressionList> arguments = make_shared<ExpressionList>();//传入参数

	NFunctionCall(){}
	NFunctionCall(const shared_ptr<NIdentifier> id,shared_ptr<ExpressionList> arguments):
	id(id),arguments(arguments){}
	NFunctionCall(const shared_ptr<NIdentifier> id):id(id){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NFunctionCall";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		this->id->print(nextPrefix);
		for(auto it=arguments->begin(); it!=arguments->end(); it++)
		{
			(*it)->print(nextPrefix);
		}
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//一元运算类
class NUnaryOperator : public NExpression
{
public:
	int op;//运算符
	shared_ptr<NExpression> expr;//表达式

	NUnaryOperator(){}
	NUnaryOperator(int op, shared_ptr<NExpression> expr):
	op(op),expr(expr){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NUnaryOperator";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << op << endl;
		expr->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};


//二元运算类
class NBinaryOperator : public NExpression
{
public:
	int op;//运算符
	shared_ptr<NExpression> lhs;//左表达式
	shared_ptr<NExpression> rhs;//右表达式

	NBinaryOperator(){}
	NBinaryOperator(shared_ptr<NExpression> lhs, int op, shared_ptr<NExpression> rhs):
	lhs(lhs),rhs(rhs),op(op){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NBinaryOperator";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << op << endl;
		lhs->print(nextPrefix);
		rhs->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//赋值类
class NAssignment : public NExpression
{
public:
	shared_ptr<NIdentifier> lhs;//左侧变量
	shared_ptr<NExpression> rhs;//右侧表达式
	
	NAssignment(){}
	NAssignment(shared_ptr<NIdentifier> lhs, shared_ptr<NExpression> rhs):
	lhs(lhs),rhs(rhs){}
	
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NAssignment";
	}
	
	//打印节点
	void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		lhs->print(nextPrefix);
		rhs->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//代码段类
class NBlock : public NExpression
{
public:
	shared_ptr<StatementList> statements = make_shared<StatementList>();//表达式向量
	
	NBlock(){}

	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NBlock";
	}
	
	//打印节点
	void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		for(auto it=statements->begin(); it!=statements->end(); it++)
		{
			(*it)->print(nextPrefix);
		}
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

/*-------------语句的子类---------------*/
//表达式语句类
class NExpressionStatement : public NStatement
{
public:
	shared_ptr<NExpression> expr;//表达式
	
	NExpressionStatement(){}
	NExpressionStatement(shared_ptr<NExpression> expr):expr(expr){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NExpressionStatement";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		expr->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//变量声明与定义类
class NVariableDeclaration : public NStatement
{
public:
	const shared_ptr<NIdentifier> type;//变量类型
	shared_ptr<NIdentifier> id;//变量名
	shared_ptr<NExpression> assignmentExpr = nullptr;//赋值语句
	
	NVariableDeclaration(){}
	NVariableDeclaration(const shared_ptr<NIdentifier> type, shared_ptr<NIdentifier> id, shared_ptr<NExpression> assignmentExpr = NULL): \
		type(type),id(id),assignmentExpr(assignmentExpr)
	{
		assert(type->isType);
		assert(!id->isArray || (id->isArray && id->arraySize != 0));
	}

	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NVariableDeclaration";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		type->print(nextPrefix);
		id->print(nextPrefix);
		if( assignmentExpr != nullptr )
		{
			assignmentExpr->print(nextPrefix);
		}
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

//函数定义类
class NFunctionDeclaration : public NStatement
{
public:
	shared_ptr<NIdentifier> type;//函数类型
	shared_ptr<NIdentifier> id;//函数名
	shared_ptr<VariableList> arguments = make_shared<VariableList>();//参数列表
	shared_ptr<NBlock> block;//代码段
	bool isExternal = false;//是否为外部函数
	
	NFunctionDeclaration(){}
	NFunctionDeclaration(shared_ptr<NIdentifier> type, shared_ptr<NIdentifier> id, shared_ptr<VariableList> arguments, shared_ptr<NBlock> block, bool isExt = false):
	type(type),id(id),arguments(arguments),block(block),isExternal(isExt)
	{
		assert(type->isType);
	}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NFunctionDeclaration";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix+this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		type->print(nextPrefix);
		id->print(nextPrefix);
		for(auto it=arguments->begin(); it!=arguments->end(); it++)
		{
			(*it)->print(nextPrefix);
		}
		assert(isExternal || block != nullptr);
		if( block)
			block->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//返回语句类
class NReturnStatement: public NStatement
{
public:
	shared_ptr<NExpression> expression;//表达式
	
	NReturnStatement(){}
	NReturnStatement(shared_ptr<NExpression> expression):expression(expression){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NReturnStatement";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix + this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		expression->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//条件语句类
class NIfStatement: public NStatement
{
public:
	shared_ptr<NExpression>  condition;//状态
	shared_ptr<NBlock> trueBlock;//为true时执行的代码段，不能为空
	shared_ptr<NBlock> falseBlock;//为false时执行的代码段，可以为空
	
	NIfStatement(){}
	NIfStatement(shared_ptr<NExpression> cond,shared_ptr<NBlock> blk,shared_ptr<NBlock> blk2 = nullptr):
	condition(cond),trueBlock(blk),falseBlock(blk2){}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NIfStatement";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix + this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
		condition->print(nextPrefix);
		trueBlock->print(nextPrefix);
		if( falseBlock )
		{
			falseBlock->print(nextPrefix);
		}
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

//循环语句类
class NIterationStatement: public NStatement
{
public:
	shared_ptr<NExpression> initial, condition, increment;//初始值、条件、增幅
	shared_ptr<NBlock> block;//执行代码段
	
	NIterationStatement(){}
	NIterationStatement(shared_ptr<NBlock> b, shared_ptr<NExpression> init = nullptr, shared_ptr<NExpression> cond = nullptr, shared_ptr<NExpression> incre = nullptr):
	block(b), initial(init), condition(cond), increment(incre)
	{
		if( condition == nullptr )
		{
			condition = make_shared<NConstant<int>>(1);
		}
	}
	//获取节点类型名
	virtual string getTypeName() const override
	{
		return "NIterationStatement";
	}
	//打印节点
	virtual void print(string prefix) const override
	{
		string nextPrefix = prefix + this->m_PREFIX;
		cout << prefix << getTypeName() << this->m_DELIM << endl;
	    	if( initial )
	    		initial->print(nextPrefix);
		if( condition )
			condition->print(nextPrefix);
		if( increment )
			increment->print(nextPrefix);
		block->print(nextPrefix);
	}
	//生成中间代码
	virtual llvm::Value *codeGen(CodeGenContext &context);

};

// 数组用下标方式访问的类
class NArrayIndex: public NExpression{
public:
    shared_ptr<NIdentifier>  arrayName;
    shared_ptr<NExpression> expression;

    NArrayIndex(){}

    NArrayIndex(shared_ptr<NIdentifier>  name, shared_ptr<NExpression> expression)
            : arrayName(name), expression(expression){
    }

    string getTypeName() const override{
        return "NArrayIndex";
    }

    void print(string prefix) const override{
        string nextPrefix = prefix + this->m_PREFIX;
        cout << prefix << getTypeName() << this->m_DELIM << endl;

        arrayName->print(nextPrefix);
		expression->print(nextPrefix);
    }
    llvm::Value *codeGen(CodeGenContext &context) override ;

};

class NArrayAssignment: public NExpression{
public:
    shared_ptr<NArrayIndex> arrayIndex;
    shared_ptr<NExpression>  expression;

    NArrayAssignment(){}

    NArrayAssignment(shared_ptr<NArrayIndex> index, shared_ptr<NExpression>  exp)
            : arrayIndex(index), expression(exp){

    }

    string getTypeName() const override{
        return "NArrayAssignment";
    }

    void print(string prefix) const override{

        string nextPrefix = prefix + this->m_PREFIX;
        cout << prefix << getTypeName() << this->m_DELIM << endl;

        arrayIndex->print(nextPrefix);
        expression->print(nextPrefix);
    }

    llvm::Value *codeGen(CodeGenContext &context) override ;

};

class NArrayInitialization: public NStatement{
public:

    NArrayInitialization(){}

    shared_ptr<NVariableDeclaration> declaration;
    shared_ptr<ExpressionList> list;

    NArrayInitialization(shared_ptr<NVariableDeclaration> dec, shared_ptr<ExpressionList> list)
            : declaration(dec), list(list){

    }

    string getTypeName() const override{
        return "NArrayInitialization";
    }

    void print(string prefix) const override{

        string nextPrefix = prefix + this->m_PREFIX;
        cout << prefix << getTypeName() << this->m_DELIM << endl;

        declaration->print(nextPrefix);
		if (list) {
			for(auto it=list->begin(); it!=list->end(); it++){
				(*it)->print(nextPrefix);
			}
		}
    }
    llvm::Value *codeGen(CodeGenContext &context) override ;

};

#endif