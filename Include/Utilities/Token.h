#ifndef _INCL_UTILITIES_TOKEN
#define _INCL_UTILITIES_TOKEN

#include <string>
#include <variant>

typedef unsigned int TOKEN_TYPE;

#define T_LPAREN '('
#define T_RPAREN ')'
#define T_LSQBRACKET '['
#define T_RSQBRACKET ']'
#define T_LCURBRACKET '{'
#define T_RCURBRACKET '}'
#define T_SEMICOLON ';'
#define T_COLON ':'
#define T_ASSIGN '=' // :=
#define T_ADD '+'
#define T_SUBTRACT '-'
#define T_MULTIPLY '*'
#define T_DIVIDE '/'
#define T_COMMA ','
#define T_AND '&'
#define T_OR '|'
#define T_PERIOD '.'

#define T_LESSTHAN 257
#define T_GREATERTHAN 258
#define T_LESSTHANEQUALTO 259
#define T_GREATERTHANEQUALTO 260
#define T_EQUALS 261 // ==
#define T_NOTEQUALS 262 // !=

#define T_IDENTIFIER 263
#define T_STRINGCONST 264
#define T_INTCONST 265
#define T_DOUBLECONST 266

#define T_EOF 267
#define T_UNKNOWN 268



struct Token
{
    TOKEN_TYPE type = T_UNKNOWN;
    std::variant<std::string, int, double> value;
    int line = 0;
    int startChar = 0;
};

typedef Token* TokenP;
typedef TokenP& TokenPR;

#endif