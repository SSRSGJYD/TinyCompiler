
#include"codegen.h"

/*-------------一些全局函数-------------*/
//输出错误
Value* LogError(string str)
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
	if(!type->name.compare("string"))
	{
		return Type::getInt8PtrTy(context.llvmContext);
	}
	return nullptr;
}

//将其他类型转换为bool类型
Value* ConvertToBoolean(CodeGenContext& context, Value* condValue)
{
	if(condValue->getType()->getTypeID() == Type::IntegerTyID)
	{
		condValue = context.builder.CreateIntCast(condValue, Type::getInt1Ty(context.llvmContext), true);
		return context.builder.CreateICmpNE(condValue, ConstantInt::get(Type::getInt1Ty(context.llvmContext), 0, true));
	}
	else if(condValue->getType()->getTypeID() == Type::FloatTyID)
	{
		return context.builder.CreateFCmpONE(condValue, ConstantFP::get(context.llvmContext, APFloat(0.0)));
	}
	else
	{
		return condValue;
	}
}

/*-------------CodeGenContext类函数实现-------------*/
CodeGenContext::CodeGenContext():builder(llvmContext)
{
	module = unique_ptr<Module>(new Module("main", this->llvmContext));
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

uint64_t CodeGenContext::getArraySize(string name) const
{
	for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++)
	{
		if((*it)->arraySize.find(name) != (*it)->arraySize.end())
		{
			return (*it)->arraySize[name];
		}
	}
	return 0;
}

void CodeGenContext::setSymbolValue(string name, Value* value)
{
	blockStack.back()->locals[name] = value;
}

void CodeGenContext::setSymbolType(string name, shared_ptr<NIdentifier> value)
{
	blockStack.back()->types[name] = value;
}

void CodeGenContext::setArraySize(string name, uint64_t value)
{
	assert(value);
	blockStack.back()->arraySize[name] = value;
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

void CodeGenContext::generateCode(NBlock& root,string filename)
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
	
	//输出中间代码
	PassManager passManager;
	error_code EC;
	raw_fd_ostream dest(filename.c_str(), EC, F_RW);
	passManager.add(createPrintModulePass(dest));
	passManager.run(*(this->module.get()));
	return;
}

void CodeGenContext::generateObj(string filename)
{
	//初始化
	InitializeAllTargetInfos();
	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmParsers();
	InitializeAllAsmPrinters();
	
	auto targetTriple = sys::getDefaultTargetTriple();
	this->module->setTargetTriple(targetTriple);
	
	string error;
	auto Target = TargetRegistry::lookupTarget(targetTriple, error);
	
	if( !Target )
	{
		errs() << error;
		return;
	}
	
	auto CPU = "generic";
	auto features = "";
	
	TargetOptions opt;
	auto RM = Optional<Reloc::Model>();
	auto theTargetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);
	
	this->module->setDataLayout(theTargetMachine->createDataLayout());
	this->module->setTargetTriple(targetTriple);
	
	error_code EC;
	raw_fd_ostream dest(filename.c_str(), EC, F_None);
	
	PassManager pass;
	auto fileType = TargetMachine::CGFT_ObjectFile;
	
	if(theTargetMachine->addPassesToEmitFile(pass, dest, fileType))
	{
		errs() << "theTargetMachine can't emit a file of this type";
		return;
	}
	
	pass.run(*(this->module.get()));
	dest.flush();
	
	cout<< "Object code wrote to " << filename.c_str() << endl;
	return;
}

bool CodeGenContext::isFuncArg(string name) const
{
	for(auto it=blockStack.rbegin(); it!=blockStack.rend(); it++)
	{
		if( (*it)->isFuncArg.find(name) != (*it)->isFuncArg.end() )
		{
			return (*it)->isFuncArg[name];
		}
	}
	return false;
}

void CodeGenContext::setFuncArg(string name, bool value)
{
	cout << "Set " << name << " as func arg" << endl;
	blockStack.back()->isFuncArg[name] = value;
}

/*-------------各节点类的codeGen()函数实现-------------*/
//常量类代码生成
template<>
Value* NConstant<int>::codeGen(CodeGenContext &context)
{
	cout << "Generating Int Constant: " << this->value << endl;
	return ConstantInt::get(Type::getInt32Ty(context.llvmContext), this->value, true);
}

template<>
Value* NConstant<float>::codeGen(CodeGenContext &context)
{
	cout << "Generating Float Constant: " << this->value << endl;
	return ConstantFP::get(Type::getFloatTy(context.llvmContext), this->value);
}

template<>
Value* NConstant<char>::codeGen(CodeGenContext &context)
{
	cout << "Generating Char Constant: '" << this->value << '\''<< endl;
	return ConstantInt::get(Type::getInt8Ty(context.llvmContext), this->value);
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
	if(value->getType()->isPointerTy())
	{
		auto arrayPtr = context.builder.CreateLoad(value, "arrayPtr");
		if(arrayPtr->getType()->isArrayTy())
		{
			cout << "(Array Type)" << endl;
			std::vector<Value*> indices;
			indices.push_back(ConstantInt::get(Type::getInt64Ty(context.llvmContext), 0));
			auto ptr = context.builder.CreateInBoundsGEP(value, indices, "arrayPtr");
			return ptr;
		}
	}
	return context.builder.CreateLoad(value, false, "");

}
bool NIdentifier::isArrayType(CodeGenContext &context)
{
	return isArray || (context.getArraySize(name) != 0);
}
//函数调用类代码生成
Value* NFunctionCall::codeGen(CodeGenContext &context)
{
	cout << "Generating function call: " << this->id->name << endl;
	Function * callFunc = context.module->getFunction(this->id->name);
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
	Value* exp = this->rhs->codeGen(context);
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
	cout << "Generating variable declaration of " << this->type->name << " " << this->id->name << (this->id->isArray ? " (Array)":" ") << endl;

	Type* type = getVarType(this->type, context);

	Value* inst = nullptr;
	if(this->id->isArray)
	{
		uint64_t arraySize = this->id->arraySize;
		if (this->isFuncArg)
		{
			inst = context.builder.CreateAlloca(PointerType::get(type, 0));
		}
		else
		{
			assert(arraySize > 0);
			Value* arraySizeValue = (NConstant<int>(arraySize)).codeGen(context);
			auto arrayType = ArrayType::get(type, arraySize);
			inst = context.builder.CreateAlloca(arrayType, arraySizeValue, "arraytmp");
		}
		context.setArraySize(this->id->name, arraySize);
	}
	else
	{
		inst = context.builder.CreateAlloca(type);
	}

	context.setSymbolType(this->id->name, this->type);
	context.setSymbolValue(this->id->name, inst);
	
	if(!this->id->isArray && this->assignmentExpr != nullptr )//赋值
	{
		NAssignment assignment(this->id, this->assignmentExpr);
		assignment.codeGen(context);
	}
	return inst;
}

//函数定义类代码生成
Value* NFunctionDeclaration::codeGen(CodeGenContext &context)
{
	cout << "Generating function declaration of " << this->id->name << endl;
	vector<Type*> argTypes;
	
	for(auto &arg: *this->arguments)
	{
		if(arg->id->isArrayType(context))
		{
			argTypes.push_back(PointerType::get(getVarType(arg->type, context), 0));
		}
		else
		{
			argTypes.push_back(getVarType(arg->type, context));
		}
	}
	Type* retType = nullptr;
	if( this->id->isArrayType(context))
		retType = PointerType::get(getVarType(this->type, context), 0);
	else
		retType = getVarType(this->type, context);
	
	FunctionType* functionType = FunctionType::get(retType, argTypes, false);
	Function* function = Function::Create(functionType, GlobalValue::ExternalLinkage, this->id->name.c_str(), context.module.get());
	
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
			(*origin_arg)->isFuncArg = true;
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

//返回语句类代码生成
Value* NReturnStatement::codeGen(CodeGenContext &context)
{
	cout << "Generating return statement" << endl;
	Value* returnValue = this->expression->codeGen(context);
	context.setReturnValue(returnValue);
	return returnValue;
}

//条件语句类代码生成
Value* NIfStatement::codeGen(CodeGenContext &context)
{
	cout << "Generating if statement" << endl;
	Value* condValue = this->condition->codeGen(context);
	
	if(!condValue)
		return nullptr;
		
	condValue = ConvertToBoolean(context, condValue);
	//获取if条件语句所在的函数
	Function* function = context.builder.GetInsertBlock()->getParent();
	
	BasicBlock *trueB = BasicBlock::Create(context.llvmContext, "then", function);
	BasicBlock *falseB = BasicBlock::Create(context.llvmContext, "else");
	BasicBlock *mergeB = BasicBlock::Create(context.llvmContext, "ifcont");//最终结束后跳转的语句块
	
	//根据有无else生成代码
	if(this->falseBlock)
	{
		context.builder.CreateCondBr(condValue, trueB, falseB);
	}
	else
	{
		context.builder.CreateCondBr(condValue, trueB, mergeB);
	}
	
	//插入if后的代码段
	context.builder.SetInsertPoint(trueB);
	context.pushBlock(trueB);
	this->trueBlock->codeGen(context);
	context.popBlock();
	trueB = context.builder.GetInsertBlock();
	
	if(!trueB->getTerminator())
	{
		context.builder.CreateBr(mergeB);
	}
	
	//插入else后的代码段
	if(this->falseBlock)
	{
		function->getBasicBlockList().push_back(falseB);
		context.builder.SetInsertPoint(falseB);
		context.pushBlock(falseB);
		this->falseBlock->codeGen(context);
		context.popBlock();
		context.builder.CreateBr(mergeB);
	}
	
	function->getBasicBlockList().push_back(mergeB);
	context.builder.SetInsertPoint(mergeB);
	
	return nullptr;
}

//循环语句类代码生成
Value* NIterationStatement::codeGen(CodeGenContext &context)
{
	Function* function = context.builder.GetInsertBlock()->getParent();
	BasicBlock *block = BasicBlock::Create(context.llvmContext, "forloop", function);
	BasicBlock *after = BasicBlock::Create(context.llvmContext, "forcont");
	
	//初始化
	if( this->initial)
		this->initial->codeGen(context);
		
	Value* condValue = this->condition->codeGen(context);
	if(!condValue)
		return nullptr;
		
	condValue = ConvertToBoolean(context, condValue);
	
	//创建条件循环
	context.builder.CreateCondBr(condValue, block, after);
	context.builder.SetInsertPoint(block);
	context.pushBlock(block);
	this->block->codeGen(context);
	context.popBlock();
	
	//更新循环条件
	if(this->increment)
	{
		this->increment->codeGen(context);
	}
	
	condValue = this->condition->codeGen(context);
	condValue = ConvertToBoolean(context, condValue);
	context.builder.CreateCondBr(condValue, block, after);
	
	function->getBasicBlockList().push_back(after);
	context.builder.SetInsertPoint(after);
	
	return nullptr;
}

// 数组索引
Value *NArrayIndex::codeGen(CodeGenContext &context)
{
	cout << "Generating array index expression of " << this->arrayName->name << endl;
	auto varPtr = context.getSymbolValue(this->arrayName->name);
	
	assert(this->arrayName->isArrayType(context));
	auto value = this->expression->codeGen(context);
	std::vector<Value*> indices;
	if(context.isFuncArg(this->arrayName->name))
	{
		cout << "isFuncArg" << endl;
		varPtr = context.builder.CreateLoad(varPtr, "actualArrayPtr");
		indices = { value };
	}
	else if(varPtr->getType()->isPointerTy())
	{
		cout << this->arrayName->name << "is not FuncArg" << endl;
		indices = { ConstantInt::get(Type::getInt64Ty(context.llvmContext), 0), value };
	}
	else
	{
		return LogError("The variable is not array");
	}
	auto ptr = context.builder.CreateInBoundsGEP(varPtr, indices, "elementPtr");
	return context.builder.CreateAlignedLoad(ptr, 4);
}

// 数组赋值
Value *NArrayAssignment::codeGen(CodeGenContext &context)
{
	cout << "Generating array index assignment of " << this->arrayIndex->arrayName->name << endl;
	auto varPtr = context.getSymbolValue(this->arrayIndex->arrayName->name);
	
	if( varPtr == nullptr )
	{
		return LogError("Unknown variable name");
	}
	
	auto arrayPtr = context.builder.CreateLoad(varPtr, "arrayPtr");
	
	if(!arrayPtr->getType()->isArrayTy() && !arrayPtr->getType()->isPointerTy())
	{
		return LogError("The variable is not array");
	}
	
	auto index = this->arrayIndex->expression->codeGen(context);
	ArrayRef<Value*> ref{ConstantInt::get(Type::getInt32Ty(context.llvmContext), 0, false), index};
	auto ptr = context.builder.CreateInBoundsGEP(varPtr, ref, "elementPtr");
	
	return context.builder.CreateAlignedStore(this->expression->codeGen(context), ptr, 4);
}

// 数组初始化
Value *NArrayInitialization::codeGen(CodeGenContext &context)
{
	cout << "Generating array initialization of " << this->declaration->id->name << endl;
	auto arrayPtr = this->declaration->codeGen(context);
	
	assert(this->declaration->id->isArray);
	
	if (this->list)
	{
		for(int index=0; index < this->list->size(); index++)
		{
			shared_ptr<NConstant<int>> indexValue = make_shared<NConstant<int>>(index);
			
			shared_ptr<NArrayIndex> arrayIndex = make_shared<NArrayIndex>(this->declaration->id, indexValue);
			NArrayAssignment assignment(arrayIndex, this->list->at(index));
			assignment.codeGen(context);
		}
	}
	return nullptr;
}

// 字符串字面量
Value *NLiteral::codeGen(CodeGenContext &context)
{
	return context.builder.CreateGlobalString(this->value, "string");
}
