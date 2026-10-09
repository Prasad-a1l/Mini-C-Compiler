%{
#include <string>
#include "AST.h"
#include "parser.hpp"

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
%}

%option noyywrap

%%

[ \t\n]                 ; // Ignore whitespace
"return"                return TOKEN(T_RETURN);
"var"                   return TOKEN(T_VAR);
"double"                return TOKEN(T_DOUBLE);
"func"                  return TOKEN(T_FUNC);
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return T_IDENTIFIER;
[0-9]+(\.[0-9]*)?       SAVE_TOKEN; return T_NUMBER;

"="                     return TOKEN(T_EQUAL);
"+"                     return TOKEN(T_PLUS);
"-"                     return TOKEN(T_MINUS);
"*"                     return TOKEN(T_MUL);
"/"                     return TOKEN(T_DIV);
"<"                     return TOKEN(T_LT);
"("                     return TOKEN(T_LPAREN);
")"                     return TOKEN(T_RPAREN);
"{"                     return TOKEN(T_LBRACE);
"}"                     return TOKEN(T_RBRACE);
","                     return TOKEN(T_COMMA);
";"                     return TOKEN(T_SEMI);

.                       printf("Unknown token! %c\n", *yytext); yyterminate();

%%
