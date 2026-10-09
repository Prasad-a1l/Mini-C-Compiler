#include "AST.h"
#include "CodeGen.h"
#include <iostream>

using namespace llvm;

Value* NumberExprAST::codegen(CodeGenContext& context) {
    return ConstantFP::get(*context.context, APFloat(Val));
}

Value* VariableExprAST::codegen(CodeGenContext& context) {
    AllocaInst* V = context.namedValues[Name];
    if (!V) {
        std::cerr << "Unknown variable name: " << Name << std::endl;
        return nullptr;
    }
    return context.builder->CreateLoad(V->getAllocatedType(), V, Name.c_str());
}

Value* BinaryExprAST::codegen(CodeGenContext& context) {
    if (Op == '=') {
        VariableExprAST* LHSE = dynamic_cast<VariableExprAST*>(LHS.get());
        if (!LHSE) {
            std::cerr << "Destination of '=' must be a variable" << std::endl;
            return nullptr;
        }
        Value* Val = RHS->codegen(context);
        if (!Val) return nullptr;
        
        AllocaInst* Variable = context.namedValues[LHSE->getName()];
        if (!Variable) {
            std::cerr << "Unknown variable name: " << LHSE->getName() << std::endl;
            return nullptr;
        }
        context.builder->CreateStore(Val, Variable);
        return Val;
    }

    Value* L = LHS->codegen(context);
    Value* R = RHS->codegen(context);
    if (!L || !R) return nullptr;

    switch (Op) {
    case '+': return context.builder->CreateFAdd(L, R, "addtmp");
    case '-': return context.builder->CreateFSub(L, R, "subtmp");
    case '*': return context.builder->CreateFMul(L, R, "multmp");
    case '<':
        L = context.builder->CreateFCmpULT(L, R, "cmptmp");
        return context.builder->CreateUIToFP(L, Type::getDoubleTy(*context.context), "booltmp");
    default:
        std::cerr << "Invalid binary operator" << std::endl;
        return nullptr;
    }
}

Value* BlockAST::codegen(CodeGenContext& context) {
    Value* last = nullptr;
    for (auto& stmt : statements) {
        last = stmt->codegen(context);
    }
    return last;
}

Value* ExprStmtAST::codegen(CodeGenContext& context) {
    return expr->codegen(context);
}

Value* VarDeclAST::codegen(CodeGenContext& context) {
    Function* TheFunction = context.builder->GetInsertBlock()->getParent();
    AllocaInst* Alloca = context.createEntryBlockAlloca(TheFunction, Name);
    
    if (InitVal) {
        Value* InitCond = InitVal->codegen(context);
        if (!InitCond) return nullptr;
        context.builder->CreateStore(InitCond, Alloca);
    }
    
    context.namedValues[Name] = Alloca;
    return Alloca;
}

Value* ReturnStmtAST::codegen(CodeGenContext& context) {
    Value* RetVal = expr->codegen(context);
    if (RetVal) {
        return context.builder->CreateRet(RetVal);
    }
    return nullptr;
}

Function* PrototypeAST::codegen(CodeGenContext& context) {
    std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*context.context));
    FunctionType* FT = FunctionType::get(Type::getDoubleTy(*context.context), Doubles, false);
    Function* F = Function::Create(FT, Function::ExternalLinkage, Name, context.module.get());

    unsigned Idx = 0;
    for (auto& Arg : F->args()) {
        Arg.setName(Args[Idx++]);
    }
    return F;
}

Function* FunctionAST::codegen(CodeGenContext& context) {
    Function* TheFunction = context.module->getFunction(Proto->getName());
    if (!TheFunction) {
        TheFunction = Proto->codegen(context);
    }
    if (!TheFunction) return nullptr;

    BasicBlock* BB = BasicBlock::Create(*context.context, "entry", TheFunction);
    context.builder->SetInsertPoint(BB);
    context.namedValues.clear();

    for (auto& Arg : TheFunction->args()) {
        AllocaInst* Alloca = context.createEntryBlockAlloca(TheFunction, std::string(Arg.getName()));
        context.builder->CreateStore(&Arg, Alloca);
        context.namedValues[std::string(Arg.getName())] = Alloca;
    }

    if (Body->codegen(context)) {
        // If there's no explicitly returned instruction but the function expects a return
        if (!context.builder->GetInsertBlock()->getTerminator()) {
            context.builder->CreateRet(ConstantFP::get(*context.context, APFloat(0.0)));
        }

        context.optimize(TheFunction);
        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}
