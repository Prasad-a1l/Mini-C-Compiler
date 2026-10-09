#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Target/TargetMachine.h>
#include <map>
#include <memory>
#include <string>

class CodeGenContext {
public:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::map<std::string, llvm::AllocaInst*> namedValues;
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm;

    CodeGenContext();
    void optimize(llvm::Function* function);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, const std::string& varName);
    void compileToAssembly(const std::string& filename, bool optimized);
};
