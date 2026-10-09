# Mini C Compiler

This is a mini compiler that compiles a C-like language subset to Assembly via LLVM IR.

## Features
- Lexical Analysis and Parsing using Flex and Bison.
- AST Generation.
- Semantic Analysis and IR Generation using LLVM.
- Optimization Passes (Constant Folding, Dead Code Elimination, Common Subexpression Elimination).
- Target Code Generation (Assembly).

## Prerequisites
- CMake 3.10+
- Flex
- Bison
- LLVM (configured and discoverable by CMake)

## Build Instructions (Windows / Linux)
1. Configure using CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   ```
2. Build the project:
   ```bash
   cmake --build .
   ```

## Usage
Run the compiler on the test file:
```bash
./minic ../test.mc
```

The optimized IR is printed to standard output, and the generated assembly code is saved to `output.s` in the current working directory.
