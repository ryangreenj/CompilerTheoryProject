#ifndef _INCL_PARSER
#define _INCL_PARSER

#include "Lexer/Lexer.h"
#include "Parser/ParseTree.h"
#include "Utilities/Error.h"
#include "Utilities/Token.h"

class Parser
{
public:
    Parser(Lexer *lexerIn);
    ParseNodeP Parse();
private:
    Lexer *m_lexer;

    ERROR_TYPE Program(Token *currToken, ParseNodePR nodeOut);
    ERROR_TYPE ProgramHeader(Token *currToken, ParseNodePR nodeOut);
    ERROR_TYPE ProgramBody(Token *currToken, ParseNodePR nodeOut);
    ERROR_TYPE Declaration(Token *currToken, ParseNodeP nodeOut);
    //ERROR_TYPE ProcedureDeclaration(Token *currToken, ParseNodeP nodeOut);
};

#endif