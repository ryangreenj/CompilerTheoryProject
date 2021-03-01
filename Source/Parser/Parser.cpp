#include "Parser/Parser.h"

#include <algorithm>

#define REQ_PARSE(func) error = func; RET_IF_ERR(error); nodeOut->children.push_back(nextNode)
#define TRY_PARSE(func) error = func; if (error == ERROR_NONE) nodeOut->children.push_back(nextNode); else if (error != ERROR_NO_OCCURRENCE) return error;
#define TRY_PARSE_MULTI(func) error = ERROR_NONE; do { error = func; if (error == ERROR_NONE) nodeOut->children.push_back(nextNode); else if(error != ERROR_NO_OCCURRENCE) return error; } while (error == ERROR_NONE)
#define IS_CERTAIN_WORD(token, word) (token->type == T_IDENTIFIER && std::get<std::string>(token->value).compare(word) == 0)
#define NEXT_TOKEN currToken = new Token(); m_lexer->GetNextToken(currToken)

const std::vector<std::string> RESERVED_WORDS
{
    "program",
    "is",
    "begin",
    "end",
    "global",
    "procedure",
    "variable",
    "type",
    "integer",
    "float",
    "string",
    "bool",
    "enum",
    "if",
    "then",
    "else",
    "for",
    "return",
    "not",
    "true",
    "false"
};

static bool IsReservedWord(std::string in)
{
    return (std::find(RESERVED_WORDS.begin(), RESERVED_WORDS.end(), in) != RESERVED_WORDS.end());
}

Parser::Parser(Lexer *lexerIn)
{
    m_lexer = lexerIn;
}

ParseNodeP Parser::Parse()
{
    TokenP currToken = new Token();
    m_lexer->GetNextToken(currToken);

    ParseNodeP nextNode = nullptr;
    ERROR_TYPE error = Program(currToken, nextNode);
    
    return error == ERROR_NONE ? nextNode : nullptr;
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
        return ERROR_NO_PROGRAM_END; // test1b.src, Maybe this should become a warning
    }
}

ERROR_TYPE Parser::ProgramHeader(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROGRAM_HEADER;

    if (!IS_CERTAIN_WORD(currToken, "program"))
    {
        return ERROR_INVALID_PROGRAM_HEADER;
    }

    NEXT_TOKEN;
    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Identifier(currToken, nextNode, true));

    if (!IS_CERTAIN_WORD(currToken, "is"))
    {
        return ERROR_INVALID_PROGRAM_HEADER;
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

    if (!IS_CERTAIN_WORD(currToken, "begin"))
    {
        return ERROR_INVALID_PROGRAM_BODY;
    }

    NEXT_TOKEN;

    nextNode = nullptr;
    TRY_PARSE_MULTI(Statement(currToken, nextNode)); // Include semicolon in Statement

    if (!IS_CERTAIN_WORD(currToken, "end"))
    {
        return ERROR_INVALID_PROGRAM_BODY;
    }

    NEXT_TOKEN;
    if (!IS_CERTAIN_WORD(currToken, "program"))
    {
        return ERROR_INVALID_PROGRAM_BODY;
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
    if (IS_CERTAIN_WORD(currToken, "global"))
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

    ParseNodeP nextNode = nullptr;
    
    TRY_PARSE(IfStatement(currToken, nextNode));

    if (error == ERROR_NO_OCCURRENCE)
    {
        nextNode = nullptr;
        TRY_PARSE(LoopStatement(currToken, nextNode));

        if (error == ERROR_NO_OCCURRENCE)
        {
            nextNode = nullptr;
            TRY_PARSE(ReturnStatement(currToken, nextNode));

            if (error == ERROR_NO_OCCURRENCE)
            {
                nextNode = nullptr;
                TRY_PARSE(AssignmentStatement(currToken, nextNode));

                if (error != ERROR_NONE)
                {
                    if (required)
                    {
                        return ERROR_INVALID_STATEMENT;
                    }
                    else
                    {
                        return error;
                    }
                }
            }
        }
    }

    // If we get here, a statement was read, currToken should be semicolon.
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

ERROR_TYPE Parser::ProcedureDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_DECLARATION;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(ProcedureHeader(currToken, nextNode));

    nextNode = nullptr;
    REQ_PARSE(ProcedureBody(currToken, nextNode, true));

    return ERROR_NONE;
}

ERROR_TYPE Parser::VariableDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::VARIABLE_DECLARATION;

    if (IS_CERTAIN_WORD(currToken, "variable"))
    {
        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Identifier(currToken, nextNode, true));

        if (currToken->type == T_COLON)
        {
            NEXT_TOKEN;

            nextNode = nullptr;
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

    return required ? ERROR_INVALID_VARIABLE_DECLARATION : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::TypeDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::TYPE_DECLARATION;

    if (IS_CERTAIN_WORD(currToken, "type"))
    {
        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Identifier(currToken, nextNode, true));

        if (IS_CERTAIN_WORD(currToken, "is"))
        {
            NEXT_TOKEN;
            nextNode = nullptr;
            REQ_PARSE(TypeMark(currToken, nextNode, true));

            return ERROR_NONE;
        }
        return ERROR_INVALID_TYPE_DECLARATION;
    }

    return required ? ERROR_INVALID_TYPE_DECLARATION : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::ProcedureHeader(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_HEADER;

    if (IS_CERTAIN_WORD(currToken, "procedure"))
    {
        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Identifier(currToken, nextNode, true));

        if (currToken->type == T_COLON)
        {
            NEXT_TOKEN;
            ParseNodeP nextNode = nullptr;
            REQ_PARSE(TypeMark(currToken, nextNode, true));

            if (currToken->type == T_LPAREN)
            {
                NEXT_TOKEN;
                nextNode = nullptr;
                TRY_PARSE(ParameterList(currToken, nextNode)); // Optional

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
    return required ? ERROR_INVALID_PROCEDURE_HEADER : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::ProcedureBody(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_BODY;

    ParseNodeP nextNode = nullptr;
    TRY_PARSE_MULTI(Declaration(currToken, nextNode));

    if (!IS_CERTAIN_WORD(currToken, "begin"))
    {
        return ERROR_INVALID_PROCEDURE_BODY;
    }

    NEXT_TOKEN;

    nextNode = nullptr;
    TRY_PARSE_MULTI(Statement(currToken, nextNode));

    if (!IS_CERTAIN_WORD(currToken, "end"))
    {
        return ERROR_INVALID_PROCEDURE_BODY;
    }

    NEXT_TOKEN;
    if (!IS_CERTAIN_WORD(currToken, "procedure"))
    {
        return ERROR_INVALID_PROCEDURE_BODY;
    }

    NEXT_TOKEN;
    return ERROR_NONE;
}

ERROR_TYPE Parser::TypeMark(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::TYPE_MARK;

    if (IS_CERTAIN_WORD(currToken, "integer") || IS_CERTAIN_WORD(currToken, "float") || IS_CERTAIN_WORD(currToken, "string") || IS_CERTAIN_WORD(currToken, "bool"))
    {
        ParseNodeP constTypeName = std::make_shared<ParseNode>();
        constTypeName->type = NodeType::SYMBOL;
        constTypeName->token = currToken;
        nodeOut->children.push_back(constTypeName);

        NEXT_TOKEN;
        return ERROR_NONE;
    }

    if (IS_CERTAIN_WORD(currToken, "enum"))
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
                ParseNodeP nextNode = nullptr;
                REQ_PARSE(Identifier(currToken, nextNode, true));

                endOfList = true;

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
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Identifier(currToken, nextNode, true));

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

        return ERROR_NONE;
    }

    return required ? ERROR_INVALID_BOUND : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::AssignmentStatement(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::ASSIGNMENT_STATEMENT;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Destination(currToken, nextNode, required));

    if (currToken->type == T_ASSIGN)
    {
        NEXT_TOKEN;

        nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, required));

        return error;
    }
    else
    {
        return ERROR_MISSING_ASSIGN;
    }

    return ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::IfStatement(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::IF_STATEMENT;

    if (!IS_CERTAIN_WORD(currToken, "if"))
    {
        return required ? ERROR_INVALID_IF_STATEMENT : ERROR_NO_OCCURRENCE;
    }
    NEXT_TOKEN;

    if (currToken->type != T_LPAREN)
    {
        return ERROR_MISSING_PAREN;
    }
    NEXT_TOKEN;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Expression(currToken, nextNode, true));

    if (currToken->type != T_RPAREN)
    {
        return ERROR_MISSING_PAREN;
    }
    NEXT_TOKEN;

    if (!IS_CERTAIN_WORD(currToken, "then"))
    {
        return ERROR_INVALID_IF_STATEMENT;
    }
    NEXT_TOKEN;

    nextNode = nullptr;
    TRY_PARSE_MULTI(Statement(currToken, nextNode));

    if (IS_CERTAIN_WORD(currToken, "else"))
    {
        ParseNodeP elseNode = std::make_shared<ParseNode>();
        elseNode->type = NodeType::SYMBOL;
        elseNode->token = currToken;
        nodeOut->children.push_back(elseNode);

        NEXT_TOKEN;

        nextNode = nullptr;
        TRY_PARSE_MULTI(Statement(currToken, nextNode));
    }

    if (IS_CERTAIN_WORD(currToken, "end"))
    {
        NEXT_TOKEN;
        if (IS_CERTAIN_WORD(currToken, "if"))
        {
            NEXT_TOKEN;
            return ERROR_NONE;
        }
    }

    return ERROR_INVALID_IF_STATEMENT;
}

ERROR_TYPE Parser::LoopStatement(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::LOOP_STATEMENT;

    if (!IS_CERTAIN_WORD(currToken, "for"))
    {
        return required ? ERROR_INVALID_IF_STATEMENT : ERROR_NO_OCCURRENCE;
    }
    NEXT_TOKEN;

    if (currToken->type != T_LPAREN)
    {
        return ERROR_MISSING_PAREN;
    }
    NEXT_TOKEN;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(AssignmentStatement(currToken, nextNode, true));

    if (currToken->type != T_SEMICOLON)
    {
        return ERROR_MISSING_SEMICOLON;
    }
    NEXT_TOKEN;

    nextNode = nullptr;
    REQ_PARSE(Expression(currToken, nextNode, true));

    if (currToken->type != T_RPAREN)
    {
        return ERROR_MISSING_PAREN;
    }
    NEXT_TOKEN;

    nextNode = nullptr;
    TRY_PARSE_MULTI(Statement(currToken, nextNode));

    if (IS_CERTAIN_WORD(currToken, "end"))
    {
        NEXT_TOKEN;
        if (IS_CERTAIN_WORD(currToken, "for"))
        {
            NEXT_TOKEN;
            return ERROR_NONE;
        }
    }

    return ERROR_INVALID_LOOP_STATEMENT;
}

ERROR_TYPE Parser::ReturnStatement(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::RETURN_STATEMENT;

    if (!IS_CERTAIN_WORD(currToken, "return"))
    {
        return required ? ERROR_INVALID_IF_STATEMENT : ERROR_NO_OCCURRENCE;
    }

    NEXT_TOKEN;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Expression(currToken, nextNode, true));

    return ERROR_NONE;
}

ERROR_TYPE Parser::ArgumentList(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::ARGUMENT_LIST;

    bool done = false;

    do
    {
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, required));

        done = true;

        if (currToken->type == T_COMMA)
        {
            done = false;
            NEXT_TOKEN;
        }
    } while (!done);

    return ERROR_NONE;
}

ERROR_TYPE Parser::Destination(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::DESTINATION;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Identifier(currToken, nextNode, required));

    if (currToken->type == T_LSQBRACKET)
    {
        NEXT_TOKEN;

        nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, true));

        if (currToken->type == T_RSQBRACKET)
        {
            NEXT_TOKEN;
            return ERROR_NONE;
        }
        return ERROR_MISSING_BRACKET;
    }

    return ERROR_NONE;
}

ERROR_TYPE Parser::Expression(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::EXPRESSION;

    bool done = true;
    bool requiredThisPass = required;

    do
    {
        if (IS_CERTAIN_WORD(currToken, "not"))
        {
            ParseNodeP notNode = std::make_shared<ParseNode>();
            notNode->type = NodeType::SYMBOL;
            notNode->token = currToken;
            nodeOut->children.push_back(notNode);
            
            requiredThisPass = true;

            NEXT_TOKEN;
        }

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(ArithOp(currToken, nextNode, requiredThisPass));
        done = true;

        if (currToken->type == T_AND || currToken->type == T_OR)
        {
            requiredThisPass = true;
            done = false;

            ParseNodeP opNode = std::make_shared<ParseNode>();
            opNode->type = NodeType::SYMBOL;
            opNode->token = currToken;
            nodeOut->children.push_back(opNode);

            NEXT_TOKEN;
        }
    } while (!done);

    return ERROR_NONE;
}

ERROR_TYPE Parser::ArithOp(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::ARITH_OP;

    bool done = true;
    bool requiredThisPass = required;

    do
    {
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Relation(currToken, nextNode, requiredThisPass));
        done = true;

        if (currToken->type == T_ADD || currToken->type == T_SUBTRACT)
        {
            requiredThisPass = true;
            done = false;

            ParseNodeP opNode = std::make_shared<ParseNode>();
            opNode->type = NodeType::SYMBOL;
            opNode->token = currToken;
            nodeOut->children.push_back(opNode);

            NEXT_TOKEN;
        }
    } while (!done);

    return ERROR_NONE;
}

ERROR_TYPE Parser::Relation(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::RELATION;

    bool done = true;
    bool requiredThisPass = required;

    do
    {
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Term(currToken, nextNode, requiredThisPass));
        done = true;

        if (currToken->type == T_LESSTHAN || currToken->type == T_GREATERTHANEQUALTO || currToken->type == T_LESSTHANEQUALTO || currToken->type == T_GREATERTHAN || currToken->type == T_EQUALS || currToken->type == T_NOTEQUALS)
        {
            requiredThisPass = true;
            done = false;

            ParseNodeP opNode = std::make_shared<ParseNode>();
            opNode->type = NodeType::SYMBOL;
            opNode->token = currToken;
            nodeOut->children.push_back(opNode);

            NEXT_TOKEN;
        }
    } while (!done);

    return ERROR_NONE;
}

ERROR_TYPE Parser::Term(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::TERM;

    bool done = true;
    bool requiredThisPass = required;

    do
    {
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Factor(currToken, nextNode, requiredThisPass));
        done = true;

        if (currToken->type == T_MULTIPLY || currToken->type == T_DIVIDE)
        {
            requiredThisPass = true;
            done = false;

            ParseNodeP opNode = std::make_shared<ParseNode>();
            opNode->type = NodeType::SYMBOL;
            opNode->token = currToken;
            nodeOut->children.push_back(opNode);

            NEXT_TOKEN;
        }
    } while (!done);

    return ERROR_NONE;
}

ERROR_TYPE Parser::Factor(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::FACTOR;

    if (currToken->type == T_LPAREN)
    {
        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, true));

        if (currToken->type == T_RPAREN)
        {
            NEXT_TOKEN;

            return ERROR_NONE;
        }
        return ERROR_MISSING_PAREN;
    }

    if (IS_CERTAIN_WORD(currToken, "true"))
    {
        currToken->value = 1;

        ParseNodeP trueNode = std::make_shared<ParseNode>();
        trueNode->type = NodeType::SYMBOL;
        trueNode->token = currToken;
        nodeOut->children.push_back(trueNode);


        NEXT_TOKEN;

        return ERROR_NONE;
    }

    if (IS_CERTAIN_WORD(currToken, "false"))
    {
        currToken->value = 0;

        ParseNodeP falseNode = std::make_shared<ParseNode>();
        falseNode->type = NodeType::SYMBOL;
        falseNode->token = currToken;
        nodeOut->children.push_back(falseNode);


        NEXT_TOKEN;

        return ERROR_NONE;
    }

    if (currToken->type == T_SUBTRACT) // Need <name> or <number>
    {
        ParseNodeP opNode = std::make_shared<ParseNode>();
        opNode->type = NodeType::SYMBOL;
        opNode->token = currToken;
        nodeOut->children.push_back(opNode);

        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        TRY_PARSE(Name(currToken, nextNode));

        if (error == ERROR_NO_OCCURRENCE)
        {
            nextNode = nullptr;
            TRY_PARSE(Number(currToken, nextNode));

            if (error != ERROR_NONE)
            {
                if (required)
                {
                    return ERROR_INVALID_FACTOR;
                }
                else
                {
                    return error;
                }
            }
            return ERROR_NONE;
        }
    }

    ParseNodeP nextNode = nullptr;
    TRY_PARSE(ProcedureCallOrName(currToken, nextNode));

    if (error == ERROR_NO_OCCURRENCE)
    {
        nextNode = nullptr;
        TRY_PARSE(Number(currToken, nextNode));

        if (error == ERROR_NO_OCCURRENCE)
        {
            nextNode = nullptr;
            TRY_PARSE(String(currToken, nextNode));

            if (error != ERROR_NONE)
            {
                if (required)
                {
                    return ERROR_INVALID_FACTOR;
                }
                else
                {
                    return error;
                }
            }
            return ERROR_NONE;
        }
    }

    return ERROR_NONE;
}

ERROR_TYPE Parser::ProcedureCallOrName(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Identifier(currToken, nextNode, required));

    if (currToken->type == T_LPAREN) // THIS IS A PROCEDURE CALL
    {
        nodeOut->type = NodeType::PROCEDURE_CALL;
        NEXT_TOKEN;

        nextNode = nullptr;
        TRY_PARSE(ArgumentList(currToken, nextNode)); // Optional

        if (currToken->type == T_RPAREN)
        {
            NEXT_TOKEN;
            return ERROR_NONE;
        }
        return ERROR_MISSING_PAREN;
    }
    else // ELSE IT'S A NAME
    {
        nodeOut->type = NodeType::NAME;

        if (currToken->type == T_LSQBRACKET)
        {
            NEXT_TOKEN;

            nextNode = nullptr;
            REQ_PARSE(Expression(currToken, nextNode, true));

            if (currToken->type == T_RSQBRACKET)
            {
                NEXT_TOKEN;
                return ERROR_NONE;
            }
            return ERROR_MISSING_BRACKET;
        }
        return ERROR_NONE;
    }

    return required ? ERROR_INVALID_PROCEDURE_CALL_OR_NAME : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::Name(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::NAME;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(Identifier(currToken, nextNode, required));

    if (currToken->type == T_LSQBRACKET)
    {
        NEXT_TOKEN;

        nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, true));

        if (currToken->type == T_RSQBRACKET)
        {
            NEXT_TOKEN;
            return ERROR_NONE;
        }
        return ERROR_MISSING_BRACKET;
    }

    return ERROR_NONE;
}

ERROR_TYPE Parser::Number(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::NUMBER;

    if ((currToken->type == T_INTCONST) || (currToken->type == T_DOUBLECONST))
    {
        nodeOut->token = currToken;
        NEXT_TOKEN;
        return ERROR_NONE;
    }

    return required ? ERROR_INVALID_STRING : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::String(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::STRING;

    if (currToken->type == T_STRINGCONST)
    {
        nodeOut->token = currToken;
        NEXT_TOKEN;
        return ERROR_NONE;
    }

    return required ? ERROR_INVALID_STRING : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::Identifier(TokenPR currToken, ParseNodePR nodeOut, bool required)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::IDENTIFIER;

    if (currToken->type == T_IDENTIFIER)
    {
        if (!IsReservedWord(std::get<std::string>(currToken->value)))
        {
            nodeOut->token = currToken;
            NEXT_TOKEN;
            return ERROR_NONE;
        }
    }

    return required ? ERROR_INVALID_IDENTIFIER : ERROR_NO_OCCURRENCE;
}