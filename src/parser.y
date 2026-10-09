%{
#include "AST.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

extern int yylex();
void yyerror(const char* s) { std::printf("Error: %s\n", s); }
FunctionAST* finalAST = nullptr;
%}

%union {
    std::string* string;
    double number;
    int token;
    ASTNode* node;
    ExprAST* expr;
    StmtAST* stmt;
    BlockAST* block;
    std::vector<std::unique_ptr<StmtAST>>* stmt_vec;
    std::vector<std::string>* args_vec;
}

%token <string> T_IDENTIFIER T_NUMBER
%token <token> T_RETURN T_VAR T_DOUBLE T_FUNC
%token <token> T_EQUAL T_PLUS T_MINUS T_MUL T_DIV T_LT
%token <token> T_LPAREN T_RPAREN T_LBRACE T_RBRACE T_COMMA T_SEMI

%type <expr> expr
%type <stmt> stmt
%type <block> block
%type <stmt_vec> stmts
%type <args_vec> func_args
%type <node> program func_decl

%left T_LT
%left T_PLUS T_MINUS
%left T_MUL T_DIV

%start program

%%

program:
    func_decl { finalAST = static_cast<FunctionAST*>($1); }
    ;

func_decl:
    T_FUNC T_IDENTIFIER T_LPAREN func_args T_RPAREN block {
        auto proto = std::make_unique<PrototypeAST>(*$2, *$4);
        delete $2;
        delete $4;
        $$ = new FunctionAST(std::move(proto), std::unique_ptr<BlockAST>($6));
    }
    ;

func_args:
    /* empty */ { $$ = new std::vector<std::string>(); }
    | T_IDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back(*$1); delete $1; }
    | func_args T_COMMA T_IDENTIFIER { $1->push_back(*$3); delete $3; $$ = $1; }
    ;

block:
    T_LBRACE stmts T_RBRACE {
        $$ = new BlockAST();
        for(auto& s : *$2) {
            $$->statements.push_back(std::move(s));
        }
        delete $2;
    }
    ;

stmts:
    /* empty */ { $$ = new std::vector<std::unique_ptr<StmtAST>>(); }
    | stmts stmt { $1->push_back(std::unique_ptr<StmtAST>($2)); $$ = $1; }
    ;

stmt:
    T_VAR T_IDENTIFIER T_EQUAL expr T_SEMI {
        $$ = new VarDeclAST(*$2, std::unique_ptr<ExprAST>($4));
        delete $2;
    }
    | T_VAR T_IDENTIFIER T_SEMI {
        $$ = new VarDeclAST(*$2, nullptr);
        delete $2;
    }
    | expr T_SEMI {
        $$ = new ExprStmtAST(std::unique_ptr<ExprAST>($1));
    }
    | T_RETURN expr T_SEMI {
        $$ = new ReturnStmtAST(std::unique_ptr<ExprAST>($2));
    }
    ;

expr:
    T_IDENTIFIER { $$ = new VariableExprAST(*$1); delete $1; }
    | T_NUMBER { $$ = new NumberExprAST(std::stod(*$1)); delete $1; }
    | expr T_PLUS expr { $$ = new BinaryExprAST('+', std::unique_ptr<ExprAST>($1), std::unique_ptr<ExprAST>($3)); }
    | expr T_MINUS expr { $$ = new BinaryExprAST('-', std::unique_ptr<ExprAST>($1), std::unique_ptr<ExprAST>($3)); }
    | expr T_MUL expr { $$ = new BinaryExprAST('*', std::unique_ptr<ExprAST>($1), std::unique_ptr<ExprAST>($3)); }
    | expr T_LT expr { $$ = new BinaryExprAST('<', std::unique_ptr<ExprAST>($1), std::unique_ptr<ExprAST>($3)); }
    | T_IDENTIFIER T_EQUAL expr { $$ = new BinaryExprAST('=', std::make_unique<VariableExprAST>(*$1), std::unique_ptr<ExprAST>($3)); delete $1; }
    ;

%%

