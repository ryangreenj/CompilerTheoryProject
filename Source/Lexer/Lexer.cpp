#include "Lexer/Lexer.h"

#include <algorithm>
#include <sstream>

#define RET_IF_SUCCESS(func) if(func) { return error; }

Lexer::Lexer() : Lexer("") {}

Lexer::Lexer(std::string inFileName)
{
    m_fileIn = new FileIn(inFileName);

    ERROR_TYPE error = ERROR_NONE;
    error = m_fileIn->LoadFile();

    if (error != ERROR_NONE)
    {
        Error::ReportError(error, "Cannot open file: " + inFileName);
    }
}

Lexer::~Lexer()
{
    delete m_fileIn;
}

ERROR_TYPE Lexer::GetNextToken(Token *&token)
{
    ERROR_TYPE error = ERROR_NONE;

    // Peek next char, then switch it
    char nextChar = '\0';
    m_fileIn->PeekChar(nextChar);

    // Consume all whitespace and comments until an actual symbol in the language
    while (ConsumeWhitespace(nextChar) || ConsumeComment(nextChar));

    // Try all token types, returning if they work
    RET_IF_SUCCESS(TrySingleCharToken(error, nextChar, token));
    RET_IF_SUCCESS(TryColonTokens(error, nextChar, token));
    RET_IF_SUCCESS(TryLTTokens(error, nextChar, token));
    RET_IF_SUCCESS(TryGTTokens(error, nextChar, token));
    RET_IF_SUCCESS(TryEqualsToken(error, nextChar, token));
    RET_IF_SUCCESS(TryNotEqualsToken(error, nextChar, token));
    RET_IF_SUCCESS(TryIdentifierToken(error, nextChar, token));
    RET_IF_SUCCESS(TryStringConstToken(error, nextChar, token));
    RET_IF_SUCCESS(TryNumericConstToken(error, nextChar, token));

    // If we get here, we aren't expecting this character.
    HandleUnexpectedToken(error, nextChar, token);

    return error;
}

bool Lexer::TrySingleCharToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    switch (currChar)
    {
    case T_LPAREN:
    case T_RPAREN:
    case T_LSQBRACKET:
    case T_RSQBRACKET:
    case T_LCURBRACKET:
    case T_RCURBRACKET:
    case T_SEMICOLON:
    case T_ADD:
    case T_SUBTRACT:
    case T_MULTIPLY:
    case T_DIVIDE: // Already checked for comment
    case T_COMMA:
    case T_AND:
    case T_OR:
    case T_PERIOD:
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        token->type = (TOKEN_TYPE)currChar;
        token->value = "";
        token->line = currLine;
        token->startChar = currLineChar;

        return true;
    }
    case '\0':
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        token->type = T_EOF;
        token->value = "";
        token->line = currLine;
        token->startChar = currLineChar;

        return true;
    }
    default: return false;
    }
}

bool Lexer::TryColonTokens(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (currChar == ':')
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        if (nextChar == '=')
        {
            m_fileIn->GetChar(currChar);

            token->type = T_ASSIGN;
        }
        else
        {
            token->type = T_COLON;
        }

        token->value = "";
        token->line = currLine;
        token->startChar = currLineChar;

        return true;
    }
    return false;
}

bool Lexer::TryLTTokens(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (currChar == '<')
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        if (nextChar == '=')
        {
            m_fileIn->GetChar(currChar);

            token->type = T_LESSTHANEQUALTO;
        }
        else
        {
            token->type = T_LESSTHAN;
        }

        token->value = "";
        token->line = currLine;
        token->startChar = currLineChar;

        return true;
    }
    return false;
}

bool Lexer::TryGTTokens(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (currChar == '>')
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        if (nextChar == '=')
        {
            m_fileIn->GetChar(currChar);

            token->type = T_GREATERTHANEQUALTO;
        }
        else
        {
            token->type = T_GREATERTHAN;
        }

        token->value = "";
        token->line = currLine;
        token->startChar = currLineChar;

        return true;
    }
    return false;
}

bool Lexer::TryEqualsToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (currChar == '=')
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        if (nextChar == '=')
        {
            m_fileIn->GetChar(currChar);

            token->type = T_EQUALS;
            token->value = "";
            token->line = currLine;
            token->startChar = currLineChar;
        }
        else
        {
            HandleUnexpectedToken(error, nextChar, token);
        }

        return true;
    }
    return false;
}

bool Lexer::TryNotEqualsToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (currChar == '!')
    {
        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        if (nextChar == '=')
        {
            m_fileIn->GetChar(currChar);

            token->type = T_EQUALS;
            token->value = "";
            token->line = currLine;
            token->startChar = currLineChar;
        }
        else
        {
            HandleUnexpectedToken(error, nextChar, token);
        }

        return true;
    }
    return false;
}

bool Lexer::TryIdentifierToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    // Identifiers start with a letter
    if (isalpha(currChar))
    {
        std::string tokenName = "";

        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);
        tokenName += currChar;

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        while (isalnum(nextChar) || nextChar == '_')
        {
            m_fileIn->GetChar(currChar);
            tokenName += currChar;
            m_fileIn->PeekChar(nextChar);
        }

        // Identifiers are case in-sensitive, convert all to lowercase
        std::transform(tokenName.begin(), tokenName.end(), tokenName.begin(), ::tolower);

        token->type = T_IDENTIFIER;
        token->value = tokenName;
        token->line = currLine;
        token->startChar = currLineChar;

        return true;
    }
    return false;
}

bool Lexer::TryStringConstToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (currChar == '"')
    {
        // Handle escape characters?
        std::string value = "";

        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        while (nextChar != '"')
        {
            m_fileIn->GetChar(currChar);
            value += currChar;
            m_fileIn->PeekChar(nextChar);
        }
        m_fileIn->GetChar(currChar);

        token->type = T_STRINGCONST;
        token->value = value;
        token->line = currLine;
        token->startChar = currLineChar;
        return true;
    }
    return false;
}

bool Lexer::TryNumericConstToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    if (isdigit(currChar))
    {
        std::string value = "";

        int currLine = 0, currLineChar = 0;
        m_fileIn->GetChar(currChar, currLine, currLineChar);
        value += currChar;

        token->type = T_INTCONST;
        token->line = currLine;
        token->startChar = currLineChar;

        char nextChar = '\0';
        m_fileIn->PeekChar(nextChar);

        while (isdigit(nextChar))
        {
            m_fileIn->GetChar(currChar);
            value += currChar;
            m_fileIn->PeekChar(nextChar);
        }

        if (nextChar == '.')
        {
            m_fileIn->GetChar(currChar);
            value += currChar;
            m_fileIn->PeekChar(nextChar);

            token->type = T_DOUBLECONST;
        }
        else
        {
            token->value = stoi(value);
            return true;
        }

        while (isdigit(nextChar))
        {
            m_fileIn->GetChar(currChar);
            value += currChar;
            m_fileIn->PeekChar(nextChar);
        }

        token->value = stod(value);
    }
    return false;
}

bool Lexer::HandleUnexpectedToken(ERROR_TYPE &error, char &currChar, Token *token)
{
    error = ERROR_UNEXPECTED_CHARACTER;
    int currLine = 0, currLineChar = 0;
    m_fileIn->GetChar(currChar, currLine, currLineChar);

    token->type = T_UNKNOWN;
    token->value = currChar;
    token->line = currLine;
    token->startChar = currLineChar;

    std::ostringstream errorMessage;
    errorMessage << "Unexcepted character '" << currChar << "' at Line " << currLine << " Pos " << currLineChar;

    Error::ReportError(error, errorMessage.str());

    return true;
}

bool Lexer::ConsumeWhitespace(char &currChar)
{
    int line, lineChar;
    bool advanced = false;

    while (IsWhitespace(currChar))
    {
        advanced = true;
        m_fileIn->AdvanceChar(line, lineChar);
        m_fileIn->PeekChar(currChar);
    }

    return advanced;
}

bool Lexer::ConsumeComment(char &currChar)
{
    int line, lineChar;
    bool advanced = false;

    if (currChar == '/')
    {
        char c;
        m_fileIn->PeekChar(c, 1);

        if (c == '/') // Single line comment, skip to end of line
        {
            advanced = true;
            do
            {
                m_fileIn->GetChar(currChar, line, lineChar);
            }
            while (currChar != '\n' && currChar != '\0');
            m_fileIn->PeekChar(currChar);
        }
        else if (c == '*') // Block comment, find end and account for nested
        {
            advanced = true;
            int levels = 1;
            char prevChar;

            m_fileIn->AdvanceChar(line, lineChar); // Advance past /
            m_fileIn->AdvanceChar(line, lineChar); // Advance past *
            m_fileIn->GetChar(currChar, line, lineChar); // Read in first char of comment
            
            while (levels > 0) // Advance until end of all nested block comments
            {
                prevChar = currChar;
                m_fileIn->GetChar(currChar, line, lineChar);

                if (currChar == '\0') // EOF
                {
                    break;
                }
                else if (prevChar == '/' && currChar == '*') // Add another level
                {
                    ++levels;
                    prevChar = currChar;
                    m_fileIn->GetChar(currChar, line, lineChar);
                }
                else if (prevChar == '*' && currChar == '/') // Close one level off
                {
                    --levels;
                    prevChar = currChar;
                    m_fileIn->GetChar(currChar, line, lineChar);
                }
            }
            m_fileIn->PeekChar(currChar);
        }
        // Else no comment, likely a division token
    }

    return advanced;
}

bool Lexer::IsWhitespace(const char &c)
{
    switch (c)
    {
    case ' ':;
    case '\t':;
    case '\n':;
    case '\r': return true;
    default: return false;
    }
}