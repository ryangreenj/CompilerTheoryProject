#ifndef _INCL_UTILITIES_TOKEN
#define _INCL_UTILITIES_TOKEN

#include <string>
#include <variant>

enum TOKEN_TYPE
{
    T_LPAREN = '(',
    T_RPAREN = ')',
    T_LSQBRACKET = '[',
    T_RSQBRACKET = ']',
    T_LCURBRACKET = '{',
    T_RCURBRACKET = '}',
    T_SEMICOLON = ';',
    T_COLON = ':',
    T_ASSIGN = '=', // :=
    T_ADD = '+',
    T_SUBTRACT = '-',
    T_MULTIPLY = '*',
    T_DIVIDE = '/',
    T_COMMA = ',',
    T_AND = '&',
    T_OR = '|',
    T_PERIOD = '.',

    T_LESSTHAN = 257,
    T_GREATERTHAN = 258,
    T_LESSTHANEQUALTO = 259,
    T_GREATERTHANEQUALTO = 260,
    T_EQUALS = 261, // ==
    T_NOTEQUALS = 262, // !=

    T_IDENTIFIER = 263,
    T_STRINGCONST = 264,
    T_INTCONST = 265,
    T_DOUBLECONST = 266,
    T_BOOLCONST = 267,

    T_EOF = 268,
    T_UNKNOWN = 269,
};

struct Token
{
    TOKEN_TYPE type = T_UNKNOWN;
    std::variant<std::string, int, double> value = "";
    int line = 0;
    int startChar = 0;
};

typedef Token* TokenP;
typedef TokenP& TokenPR;

#endif