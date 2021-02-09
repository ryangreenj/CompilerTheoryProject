#ifndef _INCL_LEXER
#define _INCL_LEXER

#include <string>

#include "Utilities/FileIn.h"
#include "Utilities/Error.h"
#include "Utilities/Token.h"

class Lexer
{
public:
    Lexer();
    Lexer(std::string inFileName);
    ~Lexer();

    ERROR_TYPE GetNextToken(Token *token);
private:
    FileIn *m_fileIn;

    bool TrySingleCharToken(char &currChar, Token *token);
    bool TryColonTokens(char &currChar, Token *token);
    bool TryLTTokens(char &currChar, Token *token);
    bool TryGTTokens(char &currChar, Token *token);
    bool TryEqualsToken(char &currChar, Token *token);
    bool TryNotEqualsToken(char &currChar, Token *token);

    bool ConsumeWhitespace(char &currChar);
    bool ConsumeComment(char &currChar);

    static bool IsWhitespace(const char &c);
};

#endif