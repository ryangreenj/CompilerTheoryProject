#include "Parser/Parser.h"

#define REQ_PARSE(func) error = func; RET_IF_ERR(error); nodeOut->children.push_back(nextNode)
#define TRY_PARSE(func) error = func; if (error == ERROR_NONE) nodeOut->children.push_back(nextNode); else if (error != ERROR_NO_OCCURRENCE) return error;
#define TRY_PARSE_MULTI(func) error = ERROR_NONE; do { error = func; if (error == ERROR_NONE) nodeOut->children.push_back(nextNode); else if(error != ERROR_NO_OCCURRENCE) return error; } while (error == ERROR_NONE)
#define IS_RESERVED_WORD(token, word) (token->type == T_IDENTIFIER && std::get<std::string>(token->value).compare(word) == 0)
#define NEXT_TOKEN currToken = new Token(); m_lexer->GetNextToken(currToken)

Parser::Parser(Lexer *lexerIn)
{
    m_lexer = lexerIn;
}

ParseNodeP Parser::Parse()
{
    TokenP currToken = new Token();
    m_lexer->GetNextToken(currToken);

    ParseNodeP nextNode = nullptr;
    int error = Program(currToken, nextNode);
    return nextNode;
}

ERROR_TYPE Parser::Program(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROGRAM;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(ProgramHeader(currToken, nextNode, true));

    nextNode = nullptr;
    REQ_PARSE(ProgramBody(currToken, nextNode, true));

    
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

ERROR_TYPE Parser::ProgramHeader(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROGRAM_HEADER;

    if (!IS_RESERVED_WORD(currToken, "program"))
    {
        return ERROR_INVALID_HEADER;
    }

    NEXT_TOKEN;
    if (currToken->type == T_IDENTIFIER)
    {
        ParseNodeP progNameNode = std::make_shared<ParseNode>();
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

ERROR_TYPE Parser::ProgramBody(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROGRAM_BODY;

    ParseNodeP nextNode = nullptr;
    TRY_PARSE_MULTI(Declaration(currToken, nextNode)); // Include semicolon in Declaration

    if (!IS_RESERVED_WORD(currToken, "begin"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;

    nextNode = nullptr;
    TRY_PARSE_MULTI(Statement(currToken, nextNode)); // Include semicolon in Statement

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

ERROR_TYPE Parser::Declaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::DECLARATION;

    bool hasGlobal = false;
    if (IS_RESERVED_WORD(currToken, "global"))
    {
        hasGlobal = true;
        ParseNodeP globalNode = std::make_shared<ParseNode>();
        globalNode->type = NodeType::SYMBOL;
        globalNode->token = currToken;
        nodeOut->children.push_back(globalNode);

        NEXT_TOKEN;
    }
    
    ParseNodeP nextNode = nullptr;
    TRY_PARSE(ProcedureDeclaration(currToken, nextNode));
    
    if (error == ERROR_NO_OCCURRENCE)
    {
        nextNode = nullptr;
        TRY_PARSE(VariableDeclaration(currToken, nextNode));

        if (error == ERROR_NO_OCCURRENCE)
        {
            nextNode = nullptr;
            TRY_PARSE(TypeDeclaration(currToken, nextNode));

            if (error != ERROR_NONE) // No occurrence
            {
                if (hasGlobal)
                {
                    return ERROR_INVALID_DECLARATION;
                }
                else
                {
                    return error;
                }
            }
        }
    }

    // If we get here, a declaration was read, currToken should be semicolon.
    if (currToken->type == T_SEMICOLON)
    {
        NEXT_TOKEN;
        return ERROR_NONE;
    }
    else
    {
        return ERROR_MISSING_SEMICOLON;
    }
}

ERROR_TYPE Parser::Statement(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::STATEMENT;

    return ERROR_NO_OCCURRENCE; // TODO: fill
}

ERROR_TYPE Parser::ProcedureDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_DECLARATION;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(ProcedureHeader(currToken, nextNode));

    nextNode = nullptr;
    REQ_PARSE(ProcedureBody(currToken, nextNode, true));
}

ERROR_TYPE Parser::VariableDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::VARIABLE_DECLARATION;

    if (IS_RESERVED_WORD(currToken, "variable"))
    {
        NEXT_TOKEN;

        if (currToken->type == T_IDENTIFIER)
        {
            ParseNodeP variableNameNode = std::make_shared<ParseNode>();
            variableNameNode->type = NodeType::IDENTIFIER;
            variableNameNode->token = currToken;
            nodeOut->children.push_back(variableNameNode);

            NEXT_TOKEN;
            if (currToken->type == T_COLON)
            {
                NEXT_TOKEN;

                ParseNodeP nextNode = nullptr;
                REQ_PARSE(TypeMark(currToken, nextNode, true));

                if (currToken->type == T_LSQBRACKET)
                {
                    NEXT_TOKEN;

                    nextNode = nullptr;
                    REQ_PARSE(Bound(currToken, nextNode, true));

                    if (currToken->type == T_RSQBRACKET)
                    {
                        NEXT_TOKEN;
                        return ERROR_NONE;
                    }
                    return ERROR_MISSING_BRACKET;
                }
                return ERROR_NONE;
            }
            return ERROR_MISSING_COLON;
        }
        return ERROR_MISSING_IDENTIFIER;
    }

    return required ? ERROR_INVALID_VARIABLE_DECLARATION : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::TypeDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::TYPE_DECLARATION;

    if (IS_RESERVED_WORD(currToken, "type"))
    {
        NEXT_TOKEN;

        if (currToken->type == T_IDENTIFIER)
        {
            ParseNodeP typeNameNode = std::make_shared<ParseNode>();
            typeNameNode->type = NodeType::IDENTIFIER;
            typeNameNode->token = currToken;
            nodeOut->children.push_back(typeNameNode);

            NEXT_TOKEN;
            if (IS_RESERVED_WORD(currToken, "is"))
            {
                NEXT_TOKEN;
                ParseNodeP nextNode = nullptr;
                REQ_PARSE(TypeMark(currToken, nextNode, true));

                return ERROR_NONE;
            }
            return ERROR_INVALID_TYPE_DECLARATION;
        }
        return ERROR_MISSING_IDENTIFIER;
    }

    return required ? ERROR_INVALID_TYPE_DECLARATION : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::ProcedureHeader(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_HEADER;

    if (IS_RESERVED_WORD(currToken, "procedure"))
    {
        NEXT_TOKEN;
        if (currToken->type == T_IDENTIFIER)
        {
            ParseNodeP procNameNode = std::make_shared<ParseNode>();
            procNameNode->type = NodeType::IDENTIFIER;
            procNameNode->token = currToken;
            nodeOut->children.push_back(procNameNode);
            NEXT_TOKEN;
            if (currToken->type == T_COLON)
            {
                NEXT_TOKEN;
                ParseNodeP nextNode = nullptr;
                REQ_PARSE(TypeMark(currToken, nextNode, true));

                if (currToken->type == T_LPAREN)
                {
                    NEXT_TOKEN;
                    nextNode = nullptr;
                    REQ_PARSE(ParameterList(currToken, nextNode, true));

                    if (currToken->type == T_RPAREN)
                    {
                        NEXT_TOKEN;
                        return ERROR_NONE;
                    }
                    return ERROR_MISSING_PAREN;
                }
                return ERROR_MISSING_PAREN;
            }
            return ERROR_MISSING_COLON;
        }
        return ERROR_MISSING_IDENTIFIER;
    }
    return ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::ProcedureBody(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_BODY;

    ParseNodeP nextNode = nullptr;
    TRY_PARSE_MULTI(Declaration(currToken, nextNode));

    if (!IS_RESERVED_WORD(currToken, "begin"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;

    nextNode = nullptr;
    TRY_PARSE_MULTI(Statement(currToken, nextNode));

    if (!IS_RESERVED_WORD(currToken, "end"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;
    if (!IS_RESERVED_WORD(currToken, "procedure"))
    {
        return ERROR_INVALID_BODY;
    }

    NEXT_TOKEN;
    return ERROR_NONE;
}

ERROR_TYPE Parser::TypeMark(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::TYPE_MARK;

    if (IS_RESERVED_WORD(currToken, "integer") || IS_RESERVED_WORD(currToken, "float") || IS_RESERVED_WORD(currToken, "string") || IS_RESERVED_WORD(currToken, "bool"))
    {
        ParseNodeP constTypeName = std::make_shared<ParseNode>();
        constTypeName->type = NodeType::SYMBOL;
        constTypeName->token = currToken;
        nodeOut->children.push_back(constTypeName);

        NEXT_TOKEN;
        return ERROR_NONE;
    }

    if (IS_RESERVED_WORD(currToken, "enum"))
    {
        ParseNodeP enumNode = std::make_shared<ParseNode>();
        enumNode->type = NodeType::SYMBOL;
        enumNode->token = currToken;
        nodeOut->children.push_back(enumNode);

        NEXT_TOKEN;

        if (currToken->type == T_LCURBRACKET)
        {
            NEXT_TOKEN;

            bool endOfList = false;

            do
            {
                if (currToken->type == T_IDENTIFIER)
                {
                    ParseNodeP identifierNode = std::make_shared<ParseNode>();
                    identifierNode->type = NodeType::IDENTIFIER;
                    identifierNode->token = currToken;
                    nodeOut->children.push_back(identifierNode);

                    endOfList = true;
                }
                else
                {
                    return ERROR_MISSING_IDENTIFIER;
                }

                NEXT_TOKEN;

                if (currToken->type == T_COMMA)
                {
                    NEXT_TOKEN;
                    endOfList = false;
                }

            } while (!endOfList);

            if (currToken->type == T_RCURBRACKET)
            {
                NEXT_TOKEN;
                return ERROR_NONE;
            }
            return ERROR_MISSING_BRACKET;
        }
        return ERROR_MISSING_BRACKET;
    }

    if (currToken->type == T_IDENTIFIER)
    {
        ParseNodeP identifierNode = std::make_shared<ParseNode>();
        identifierNode->type = NodeType::IDENTIFIER;
        identifierNode->token = currToken;
        nodeOut->children.push_back(identifierNode);

        NEXT_TOKEN;
        return ERROR_NONE;
    }

    return required ? ERROR_INVALID_TYPE_MARK : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::ParameterList(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PARAMETER_LIST;

    bool done = false;

    do
    {
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Parameter(currToken, nextNode, required));

        done = true;

        if (currToken->type == T_COMMA)
        {
            done = false;
            NEXT_TOKEN;
        }
    } while (!done);

    return ERROR_NONE;
}

ERROR_TYPE Parser::Parameter(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PARAMETER;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(VariableDeclaration(currToken, nextNode, required));

    return ERROR_NONE;
}

ERROR_TYPE Parser::Bound(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::BOUND;

    if (currToken->type == T_INTCONST)
    {
        nodeOut->token = currToken;
        NEXT_TOKEN;
    }

    return required ? ERROR_INVALID_BOUND : ERROR_NO_OCCURRENCE;
}