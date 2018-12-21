
#include"codegen.h"

/*-------------一些全局函数-------------*/
unique_ptr<NExpression> LogError(string str)
{
	cout<<"LogError: "<<str<<endl;
	return nullptr;
}

/*-------------CodeGenContext类函数实现-------------*/
CodeGenContext::CodeGenContext():builder(llvmContext)
{
	theModule = unique_ptr<Module>(new Module("main", this->llvmContext));
}
    
Value* CodeGenContext::getSymbolValue(string name) const
{
	for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++)
	{
		if((*it)->locals.find(name) != (*it)->locals.end())
		{
			return (*it)->locals[name];
		}
	}
	return nullptr;
}

shared_ptr<NIdentifier> CodeGenContext::getSymbolType(string name) const
{
	for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++)
	{
		if((*it)->types.find(name) != (*it)->types.end())
		{
			return (*it)->types[name];
		}
	}
	return nullptr;
}

void CodeGenContext::setSymbolValue(string name, Value* value)
{
	blockStack.back()->locals[name] = value;
}

void CodeGenContext::setSymbolType(string name, shared_ptr<NIdentifier> value)
{
	blockStack.back()->types[name] = value;
}

BasicBlock* CodeGenContext::currentBlock() const
{
	return blockStack.back()->block;
}

void CodeGenContext::pushBlock(BasicBlock * block)
{
	CodeGenBlock * codeGenBlock = new CodeGenBlock();
	codeGenBlock->block = block;
	codeGenBlock->returnValue = nullptr;
	blockStack.push_back(codeGenBlock);
}

void CodeGenContext::popBlock()
{
	CodeGenBlock * codeGenBlock = blockStack.back();
	blockStack.pop_back();
	delete codeGenBlock;
}

void CodeGenContext::setReturnValue(Value* value)
{
	blockStack.back()->returnValue = value;
}

Value* CodeGenContext::getReturnValue()
{
	return blockStack.back()->returnValue;
}

void CodeGenContext::generateCode(NBlock& root)
{
	cout << "Generating llvm IR code..." << endl;
	//创建顶层系统调用函数作为入口
	vector<Type*> argTypes;
	FunctionType* mainFuncType = FunctionType::get(Type::getVoidTy(this->llvmContext), makeArrayRef(argTypes), false);
    Function* mainFunction = Function::Create(mainFuncType, GlobalValue::ExternalLinkage, "main");
    BasicBlock* block = BasicBlock::Create(this->llvmContext,"entry");

	//从主函数进入开始解析
    pushBlock(block);
    Value* retValue = root.codeGen(*this);
    popBlock();

    cout << "Generating successfully！" << endl;

    PassManager passManager;
    passManager.add(createPrintModulePass(outs()));
    passManager.run(*(this->theModule.get()));
    return;
}
/*-------------各节点类的codeGen()函数实现-------------*/
//常量类代码生成
template<class T>
Value* NConstant<T>::codeGen(CodeGenContext &context)
{
    cout << "Generating Constant: " << this->value << endl;
    if(typeid(this->value) == typeid(int))
    {
    	return ConstantInt::get(Type::getInt32Ty(context.llvmContext), this->value, true);
    }
    else if(typeid(this->value) == typeid(float))
    {
    	return ConstantFP::get(Type::getFloatTy(context.llvmContext), this->value);
    }

}

//变量类代码生成
Value* NIdentifier::codeGen(CodeGenContext &context)
{
    cout << "Generating identifier: " << this->name << endl;
    Value* value = context.getSymbolValue(this->name);
    //变量值不存在
    if(!value)
    {
        return LogError("Unknown variable name " + this->name);
    }
    return context.builder.CreateLoad(value, false, "");

}

//函数调用类代码生成
Value* NFunctionCall::codeGen(CodeGenContext &context)
{
	cout << "Generating function call: " << this->id->name << endl;
    Function * callFunc = context.theModule->getFunction(this->id->name);
    //没有函数的定义
    if(!callFunc)
    {
        LogError("Function name not found");
    }
    //参数类型不匹配
    if(callFunc->arg_size() != this->arguments->size())
    {
        LogError("Function arguments size not match, callFunc=" + to_string(callFunc->size()) + ", this->arguments=" + to_string(this->arguments->size()));
    }
    vector<Value*> argsv;//参数列表
    for(auto it=this->arguments->begin(); it!=this->arguments->end();it++)
    {
    	argsv.push_back((*it)->codeGen(context));
        if(!argsv.back())//参数压入失败
        {
            return nullptr;
        }
    }
    return context.builder.CreateCall(callFunc, argsv, "callFunc");
}

//二元运算类代码生成
Value* NBinaryOperator::codeGen(CodeGenContext &context)
{
	cout << "Generating binary operator" << endl;
	Value* L = this->lhs->codeGen(context);
	Value* R = this->rhs->codeGen(context);
	bool fp = false;//是否为浮点数计算
	
	if((L->getType()->getTypeID() == Type::FloatTyID) || (R->getType()->getTypeID() == Type::DoubleTyID) ){  // type upgrade
        fp = true;
        if( (R->getType()->getTypeID() != Type::DoubleTyID) ){
            R = context.builder.CreateUIToFP(R, Type::getDoubleTy(context.llvmContext), "ftmp");
        }
        if( (L->getType()->getTypeID() != Type::DoubleTyID) ){
            L = context.builder.CreateUIToFP(L, Type::getDoubleTy(context.llvmContext), "ftmp");
        }
    }

    if( !L || !R ){
        return nullptr;
    }
    cout << "fp = " << ( fp ? "true" : "false" ) << endl;
    cout << "L is " << TypeSystem::llvmTypeToStr(L) << endl;
    cout << "R is " << TypeSystem::llvmTypeToStr(R) << endl;

    switch (this->op){
        case TPLUS:
            return fp ? context.builder.CreateFAdd(L, R, "addftmp") : context.builder.CreateAdd(L, R, "addtmp");
        case TMINUS:
            return fp ? context.builder.CreateFSub(L, R, "subftmp") : context.builder.CreateSub(L, R, "subtmp");
        case TMUL:
            return fp ? context.builder.CreateFMul(L, R, "mulftmp") : context.builder.CreateMul(L, R, "multmp");
        case TDIV:
            return fp ? context.builder.CreateFDiv(L, R, "divftmp") : context.builder.CreateSDiv(L, R, "divtmp");
        case TAND:
            return fp ? LogErrorV("Double type has no AND operation") : context.builder.CreateAnd(L, R, "andtmp");
        case TOR:
            return fp ? LogErrorV("Double type has no OR operation") : context.builder.CreateOr(L, R, "ortmp");
        case TXOR:
            return fp ? LogErrorV("Double type has no XOR operation") : context.builder.CreateXor(L, R, "xortmp");
        case TSHIFTL:
            return fp ? LogErrorV("Double type has no LEFT SHIFT operation") : context.builder.CreateShl(L, R, "shltmp");
        case TSHIFTR:
            return fp ? LogErrorV("Double type has no RIGHT SHIFT operation") : context.builder.CreateAShr(L, R, "ashrtmp");

        case TCLT:
            return fp ? context.builder.CreateFCmpULT(L, R, "cmpftmp") : context.builder.CreateICmpULT(L, R, "cmptmp");
        case TCLE:
            return fp ? context.builder.CreateFCmpOLE(L, R, "cmpftmp") : context.builder.CreateICmpSLE(L, R, "cmptmp");
        case TCGE:
            return fp ? context.builder.CreateFCmpOGE(L, R, "cmpftmp") : context.builder.CreateICmpSGE(L, R, "cmptmp");
        case TCGT:
            return fp ? context.builder.CreateFCmpOGT(L, R, "cmpftmp") : context.builder.CreateICmpSGT(L, R, "cmptmp");
        case TCEQ:
            return fp ? context.builder.CreateFCmpOEQ(L, R, "cmpftmp") : context.builder.CreateICmpEQ(L, R, "cmptmp");
        case TCNE:
            return fp ? context.builder.CreateFCmpONE(L, R, "cmpftmp") : context.builder.CreateICmpNE(L, R, "cmptmp");
        default:
            return LogErrorV("Unknown binary operator");
    }
}


