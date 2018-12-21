
#include"codegen.h"

/*-------------一些全局函数-------------*/
//输出错误
unique_ptr<NExpression> LogError(string str)
{
	cout<<"LogError: "<<str<<endl;
	return nullptr;
}

//返回string类型的type
string llvmTypeToStr(Value *value)
{
    Type::TypeID typeID;        
    if(value)
        typeID = value->getType()->getTypeID();
    else
        return "Value is nullptr";

    switch (typeID)
    {
    	case Type::VoidTyID:
    		return "VoidTyID";
    	case Type::HalfTyID:
    		return "HalfTyID";
        case Type::FloatTyID:
            return "FloatTyID";
        case Type::DoubleTyID:
            return "DoubleTyID";
        case Type::IntegerTyID:
            return "IntegerTyID";
        case Type::FunctionTyID:
            return "FunctionTyID";
        case Type::StructTyID:
            return "StructTyID";
        case Type::ArrayTyID:
            return "ArrayTyID";
        case Type::PointerTyID:
            return "PointerTyID";
        case Type::VectorTyID:
            return "VectorTyID";
        default:
            return "Unknown";
    }
}

//返回Type类型的type
Type *getVarType(shared_ptr<NIdentifier> type,CodeGenContext &context)
{
	assert(type->isType);
	if(!type->name.compare("int"))
	{
        return Type::getInt32Ty(context.llvmContext);
    }
    if(!type->name.compare("float"))
    {
        return Type::getFloatTy(context.llvmContext);
    }
    if(!type->name.compare("char"))
    {
        return Type::getInt8Ty(context.llvmContext);
    }
    if(!type->name.compare("void"))
    {
        return Type::getVoidTy(context.llvmContext);
    }

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

//一元运算类代码生成
Value* NUnaryOperator::codeGen(CodeGenContext &context)
{
	cout << "Generating unary operator" << endl;
	Value* L = this->expr->codeGen(context);
	Value* R = ConstantInt::get(Type::getInt32Ty(context.llvmContext),1, true);
	bool fp = false;//是否为浮点数计算

    if(!L)
    {
        return nullptr;
    }
    
	//判断是否为浮点数记算
	if((L->getType()->getTypeID() == Type::FloatTyID))
	{
		fp = true;
    }
    
    //判断一元运算类型
    switch (this->op)
    {
        case T_INC:
            return fp ? LogError("Float type has no INC operation") : context.builder.CreateAdd(L, R, "addtmp");
        case T_DEC:
            return fp ? LogError("Float type has no DEC operation") : context.builder.CreateSub(L, R, "subtmp");
        case T_NOT:
            return context.builder.CreateNot(L, "nottmp");
		default:
			return LogError("Unknown unary operator");
    }
}

//二元运算类代码生成
Value* NBinaryOperator::codeGen(CodeGenContext &context)
{
	cout << "Generating binary operator" << endl;
	Value* L = this->lhs->codeGen(context);
	Value* R = this->rhs->codeGen(context);
	bool fp = false;//是否为浮点数计算
	
	//若是浮点数计算，则统一形式
	if((L->getType()->getTypeID() == Type::FloatTyID) || (R->getType()->getTypeID() == Type::FloatTyID))
	{
		fp = true;
		if((R->getType()->getTypeID() != Type::FloatTyID))
		{
            R = context.builder.CreateUIToFP(R, Type::getFloatTy(context.llvmContext),"ftmp");
        }
        if((L->getType()->getTypeID() != Type::FloatTyID))
        {
            L = context.builder.CreateUIToFP(L, Type::getFloatTy(context.llvmContext),"ftmp");
        }
    }

    if( !L || !R )
    {
        return nullptr;
    }
    cout << "fp = " << ( fp ? "true" : "false" ) << endl;
    cout << "L = " << llvmTypeToStr(L) << endl;
    cout << "R = " << llvmTypeToStr(R) << endl;
    
    //判断二元运算类型
    switch (this->op)
    {
    	case T_CMP_EQ:
    		return fp ? context.builder.CreateFCmpOEQ(L, R, "cmpftmp") : context.builder.CreateICmpEQ(L, R, "cmptmp");
    	case T_CMP_GE:
    		return fp ? context.builder.CreateFCmpOGE(L, R, "cmpftmp") : context.builder.CreateICmpSGE(L, R, "cmptmp");
    	case T_CMP_GT:
    		return fp ? context.builder.CreateFCmpOGT(L, R, "cmpftmp") : context.builder.CreateICmpSGT(L, R, "cmptmp");
    	case T_CMP_LE:
    		return fp ? context.builder.CreateFCmpOLE(L, R, "cmpftmp") : context.builder.CreateICmpSLE(L, R, "cmptmp");
    	case T_CMP_LT:
    		return fp ? context.builder.CreateFCmpULT(L, R, "cmpftmp") : context.builder.CreateICmpULT(L, R, "cmptmp");
    	case T_CMP_NE:
    		return fp ? context.builder.CreateFCmpONE(L, R, "cmpftmp") : context.builder.CreateICmpNE(L, R, "cmptmp");
        case T_OR:
            return fp ? LogError("Float type has no OR operation") : context.builder.CreateOr(L, R, "ortmp");
        case T_XOR:
            return fp ? LogError("Float type has no XOR operation") : context.builder.CreateXor(L, R, "xortmp");
        case T_AND:
            return fp ? LogError("Float type has no AND operation") : context.builder.CreateAnd(L, R, "andtmp");
        case T_MUL:
            return fp ? context.builder.CreateFMul(L, R, "mulftmp") : context.builder.CreateMul(L, R, "multmp");
        case T_DIV:
            return fp ? context.builder.CreateFDiv(L, R, "divftmp") : context.builder.CreateSDiv(L, R, "divtmp");
    	case T_ADD:
    		return fp ? context.builder.CreateFAdd(L, R, "addftmp") : context.builder.CreateAdd(L, R, "addtmp");
        case T_SUB:
            return fp ? context.builder.CreateFSub(L, R, "subftmp") : context.builder.CreateSub(L, R, "subtmp");
		default:
			return LogError("Unknown binary operator");
    }
}

//赋值类代码生成
Value* NAssignment::codeGen(CodeGenContext &context)
{
	cout << "Generating assignment of " << this->lhs->name << endl;
    Value* dst = context.getSymbolValue(this->lhs->name);
    string dstTypeStr = context.getSymbolType(this->lhs->name)->name;
    
    if(!dst)
    {
    	return LogError("Undeclared variable");
    }
    Value* exp = exp = this->rhs->codeGen(context);
    context.builder.CreateStore(exp,dst);
    return dst;
}

//代码段类代码生成
Value* NBlock::codeGen(CodeGenContext &context)
{
	cout << "Generating block" << endl;
	Value* last = nullptr;
	for(auto it=this->statements->begin(); it!=this->statements->end(); it++)
	{
		last = (*it)->codeGen(context);
    }
    return last;
}

//表达式语句类代码生成
Value* NExpressionStatement::codeGen(CodeGenContext &context)
{
	return this->expr->codeGen(context);
}

//变量声明与定义类代码生成
Value* NVariableDeclaration::codeGen(CodeGenContext &context)
{
    cout << "Generating variable declaration of " << this->type->name << " " << this->id->name << endl;
    Type* type = getVarType(this->type, context);

    Value* alloca = nullptr;//分配空间
    
    alloca = context.builder.CreateAlloca(type);

    context.setSymbolType(this->id->name, this->type);
    context.setSymbolValue(this->id->name, alloca);
    
    if( this->assignmentExpr != nullptr )//赋值
    {
    	NAssignment assignment(this->id, this->assignmentExpr);
        assignment.codeGen(context);
    }
    return alloca;
}

//函数定义类代码生成
Value* NFunctionDeclaration::codeGen(CodeGenContext &context)
{
	cout << "Generating function declaration of " << this->id->name << endl;
	vector<Type*> argTypes;
	
	for(auto &arg: *this->arguments)
	{
		argTypes.push_back(getVarType(arg->type, context));
	}
    Type* retType = getVarType(this->type, context);
    
    FunctionType* functionType = FunctionType::get(retType, argTypes, false);
    Function* function = Function::Create(functionType, GlobalValue::ExternalLinkage, this->id->name.c_str(), context.theModule.get());

    if(!this->isExternal)//不是引用的外部函数
    {
    	//创建代码段
    	BasicBlock* basicBlock = BasicBlock::Create(context.llvmContext, "entry", function, nullptr);
    	
    	context.builder.SetInsertPoint(basicBlock);
        context.pushBlock(basicBlock);

		//函数参数设置
        auto origin_arg = this->arguments->begin();
        for(auto &arg: function->args())
        {
        	arg.setName((*origin_arg)->id->name);
            Value* argAlloc;
            argAlloc = (*origin_arg)->codeGen(context);

            context.builder.CreateStore(&arg, argAlloc, false);
            context.setSymbolValue((*origin_arg)->id->name, argAlloc);
            context.setSymbolType((*origin_arg)->id->name, (*origin_arg)->type);
            context.setFuncArg((*origin_arg)->id->name, true);
            origin_arg++;
        }
		
		//代码段与返回值
        this->block->codeGen(context);
        if( context.getReturnValue())
        {
            context.builder.CreateRet(context.getReturnValue());
        }
        else
        {
            return LogError("Function block return value not found");
        }
        context.popBlock();
    }

    return function;
}


