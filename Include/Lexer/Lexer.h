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

    bool TrySingleCharToken(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryColonTokens(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryLTTokens(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryGTTokens(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryEqualsToken(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryNotEqualsToken(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryIdentifierToken(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryStringConstToken(ERROR_TYPE &error, char &currChar, Token *token);
    bool TryNumericConstToken(ERROR_TYPE &error, char &currChar, Token *token);

    bool HandleUnexpectedToken(ERROR_TYPE &error, char &currChar, Token *token);

    bool ConsumeWhitespace(char &currChar);
    bool ConsumeComment(char &currChar);

    static bool IsWhitespace(const char &c);
};

#endif