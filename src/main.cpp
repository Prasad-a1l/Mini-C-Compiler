#include <iostream>
#include "AST.h"
#include "CodeGen.h"

extern FILE* yyin;
extern int yyparse();
extern FunctionAST* finalAST;

int main(int argc, char** argv) {
    if (argc > 1) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            std::cerr << "Cannot open file " << argv[1] << std::endl;
            return 1;
        }
        yyin = file;
    } else {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }

    yyparse();

    if (finalAST) {
        CodeGenContext context;
        finalAST->codegen(context);
        
        // Output IR
        std::cout << "--- Optimized LLVM IR ---" << std::endl;
        context.module->print(llvm::outs(), nullptr);
        std::cout << "-------------------------" << std::endl;
        
        // Generate assembly
        context.compileToAssembly("output.s", true); // Pass optimized=true
        std::cout << "Assembly output written to output.s (optimized)" << std::endl;
    } else {
        std::cerr << "Failed to parse AST." << std::endl;
    }

    return 0;
}
