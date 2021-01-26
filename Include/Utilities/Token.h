#ifndef _INCL_UTILITIES_TOKEN
#define _INCL_UTILITIES_TOKEN

#include <string>

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

#define T_LESSTHAN 257
#define T_GREATERTHAN 258
#define T_LESSTHANEQUALTO 259
#define T_GREATERTHANEQUALTO 260
#define T_EQUALS 261
#define T_NOTEQUALS 262



struct Token
{
    TOKEN_TYPE type;
    union
    {
        std::string stringValue;
        int intValue;
        double doubleValue;
    };
    int line;
    int startChar;
};

#endif