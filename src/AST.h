#pragma once
#include <string>
#include <vector>
#include <memory>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

class CodeGenContext;

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class ExprAST : public ASTNode {
public:
    virtual llvm::Value* codegen(CodeGenContext& context) = 0;
};

class StmtAST : public ASTNode {
public:
    virtual llvm::Value* codegen(CodeGenContext& context) = 0;
};

class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(double Val) : Val(Val) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

class VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string& Name) : Name(Name) {}
    const std::string& getName() const { return Name; }
    llvm::Value* codegen(CodeGenContext& context) override;
};

class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

class BlockAST : public ExprAST {
public:
    std::vector<std::unique_ptr<StmtAST>> statements;
    BlockAST() {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

class ExprStmtAST : public StmtAST {
    std::unique_ptr<ExprAST> expr;
public:
    ExprStmtAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

class VarDeclAST : public StmtAST {
    std::string Name;
    std::unique_ptr<ExprAST> InitVal;
public:
    VarDeclAST(const std::string& Name, std::unique_ptr<ExprAST> InitVal)
        : Name(Name), InitVal(std::move(InitVal)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

class ReturnStmtAST : public StmtAST {
    std::unique_ptr<ExprAST> expr;
public:
    ReturnStmtAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)) {}
    llvm::Value* codegen(CodeGenContext& context) override;
};

class PrototypeAST : public ASTNode {
    std::string Name;
    std::vector<std::string> Args;
public:
    PrototypeAST(const std::string& Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
    const std::string& getName() const { return Name; }
    llvm::Function* codegen(CodeGenContext& context);
};

class FunctionAST : public ASTNode {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<BlockAST> Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<BlockAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
    llvm::Function* codegen(CodeGenContext& context);
};
