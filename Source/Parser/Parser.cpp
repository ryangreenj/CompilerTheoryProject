#include "Parser/Parser.h"

#define REQ_PARSE(func) RET_IF_ERR(func); nodeOut->children.push_back(nextNode);
#define TRY_PARSE(func) ERROR_TYPE errorLocal = func; if (errorLocal == ERROR_NONE) nodeOut->children.push_back(nextNode); else if (errorLocal != ERROR_NO_OCCURRENCE) return errorLocal;
#define TRY_PARSE_MULTI(func) ERROR_TYPE errorLocal = ERROR_NONE; do { errorLocal = func; if (errorLocal != ERROR_NONE && errorLocal != ERROR_NO_OCCURRENCE) return errorLocal; nodeOut->children.push_back(nextNode); } while (func == ERROR_NONE);
#define IS_RESERVED_WORD(token, word) token->type == T_IDENTIFIER && std::get<std::string>(token->value).compare(word)
#define NEXT_TOKEN currToken = new Token(); m_lexer->GetNextToken(currToken);

Parser::Parser(Lexer *lexerIn)
{
    m_lexer = lexerIn;
}

ParseNodeP Parser::Parse()
{
    Token *currToken = new Token();
    m_lexer->GetNextToken(currToken);

    ParseNodeP nextNode = nullptr;
    int error = Program(currToken, nextNode);
    return nextNode;
}

ERROR_TYPE Parser::Program(Token *currToken, ParseNodePR nodeOut)
{
    nodeOut = new ParseNode();
    nodeOut->type = NodeType::PROGRAM;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(ProgramHeader(currToken, nextNode));

    nextNode = nullptr;
    REQ_PARSE(ProgramBody(currToken, nextNode));

    
    if (currToken->type == T_PERIOD)
    {
        NEXT_TOKEN;
        return ERROR_NONE;
    }
    else
    {
        return ERROR_NO_PROGRAM_END;
    }
}

ERROR_TYPE Parser::ProgramHeader(Token *currToken, ParseNodePR nodeOut)
{
    nodeOut = new ParseNode();
    nodeOut->type = NodeType::PROGRAM_HEADER;

    if (!IS_RESERVED_WORD(currToken, "program"))
    {
        return ERROR_INVALID_HEADER;
    }

    NEXT_TOKEN;
    if (currToken->type == T_IDENTIFIER)
    {
        ParseNodeP progNameNode = new ParseNode();
        progNameNode->type = NodeType::IDENTIFIER;
        progNameNode->token = currToken;
        nodeOut->children.push_back(progNameNode);
    }

    NEXT_TOKEN;
    if (!IS_RESERVED_WORD(currToken, "is"))
    {
        return ERROR_INVALID_HEADER;
    }

    NEXT_TOKEN;
    return ERROR_NONE;
}

ERROR_TYPE Parser::ProgramBody(Token *currToken, ParseNodePR nodeOut)
{
    nodeOut = new ParseNode();
    nodeOut->type = NodeType::PROGRAM_BODY;

    ParseNodeP nextNode = nullptr;
    TRY_PARSE_MULTI(Declaration(currToken, nextNode));  // Include semicolon in Declaration

    if (!IS_RESERVED_WORD(currToken, "begin"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;
    if (!IS_RESERVED_WORD(currToken, "end"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;
    if (!IS_RESERVED_WORD(currToken, "program"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;
    return ERROR_NONE;
}