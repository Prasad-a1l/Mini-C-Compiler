#include "CodeGen.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <iostream>

CodeGenContext::CodeGenContext() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("MiniCModule", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    // Initialize PassManager
    fpm = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());
    
    // Constant Folding, Dead Code Elimination, Common Subexpression Elimination
    fpm->add(llvm::createInstructionCombiningPass());
    fpm->add(llvm::createReassociatePass());
    fpm->add(llvm::createGVNPass());
    fpm->add(llvm::createCFGSimplificationPass());
    fpm->doInitialization();
}

llvm::AllocaInst* CodeGenContext::createEntryBlockAlloca(llvm::Function* function, const std::string& varName) {
    llvm::IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*context), nullptr, varName);
}

void CodeGenContext::optimize(llvm::Function* function) {
    fpm->run(*function);
}

void CodeGenContext::compileToAssembly(const std::string& filename, bool optimized) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        llvm::errs() << Error;
        return;
    }

    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>(); // use std::optional for modern llvm, or remove if unsupported
    
    // Most recent LLVM uses std::optional.
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    
    module->setDataLayout(TargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_AssemblyFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module);
    dest.flush();
}
