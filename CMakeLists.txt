cmake_minimum_required(VERSION 3.10)
project(MiniCCompiler)

set(CMAKE_CXX_STANDARD 17)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add the src folder to include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Flex and Bison
FLEX_TARGET(Lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
BISON_TARGET(Parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp COMPILE_FLAGS "-d")
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

set(SOURCES
    src/main.cpp
    src/AST.cpp
    src/CodeGen.cpp
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
)

add_executable(minic ${SOURCES})

llvm_map_components_to_libnames(llvm_libs core irreader bitwriter passes linker target native support)
target_link_libraries(minic ${llvm_libs})
