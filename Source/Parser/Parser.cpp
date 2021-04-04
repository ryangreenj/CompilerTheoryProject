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
    m_symbolTable = new SymbolTable();

    // TEMPORARILY ADD RUNTIME PROCEDURES INTO SYMBOL TABLE
    m_symbolTable->InsertGlobal("getbool", ValueType::BOOL, true, 0);
    m_symbolTable->InsertGlobal("getinteger", ValueType::INT, true, 0);
    m_symbolTable->InsertGlobal("getfloat", ValueType::DOUBLE, true, 0);
    m_symbolTable->InsertGlobal("getstring", ValueType::STRING, true, 0);
    m_symbolTable->InsertGlobal("putbool", ValueType::BOOL, true, 0);
    m_symbolTable->InsertGlobal("putinteger", ValueType::BOOL, true, 0);
    m_symbolTable->InsertGlobal("putfloat", ValueType::BOOL, true, 0);
    m_symbolTable->InsertGlobal("putstring", ValueType::BOOL, true, 0);
    m_symbolTable->InsertGlobal("sqrt", ValueType::DOUBLE, true, 0);
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
    TRY_PARSE(ProcedureDeclaration(currToken, nextNode, false, hasGlobal));
    
    if (error == ERROR_NO_OCCURRENCE)
    {
        nextNode = nullptr;
        TRY_PARSE(VariableDeclaration(currToken, nextNode, false, hasGlobal));

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

ERROR_TYPE Parser::ProcedureDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required, bool hasGlobal)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_DECLARATION;

    ParseNodeP nextNode = nullptr;
    REQ_PARSE(ProcedureHeader(currToken, nextNode, false, hasGlobal));

    nextNode = nullptr;
    REQ_PARSE(ProcedureBody(currToken, nextNode, true));

    RET_IF_ERR(m_symbolTable->DeleteLevel()); // Delete scope after the procedure body

    return ERROR_NONE;
}

ERROR_TYPE Parser::VariableDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required, bool hasGlobal)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::VARIABLE_DECLARATION;

    if (IS_CERTAIN_WORD(currToken, "variable"))
    {
        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Identifier(currToken, nextNode, true));

        // Save ident for adding to symbol table
        std::string ident = std::get<std::string>(nextNode->token->value);

        if (currToken->type == T_COLON)
        {
            NEXT_TOKEN;

            nextNode = nullptr;
            REQ_PARSE(TypeMark(currToken, nextNode, true));

            ValueType nodeType = nextNode->valueType;

            nodeOut->valueType = nodeType;

            if (currToken->type == T_LSQBRACKET)
            {
                NEXT_TOKEN;

                nextNode = nullptr;
                REQ_PARSE(Bound(currToken, nextNode, true));

                int bound = std::get<int>(nextNode->token->value);

                // Insert into symbol table
                nodeType = (ValueType) ((int)nodeType + NOT_TO_ARRAY);

                nodeOut->valueType = nodeType;

                if (hasGlobal)
                {
                    RET_IF_ERR(m_symbolTable->InsertGlobal(ident, nodeType, false, bound));
                }
                else
                {
                    RET_IF_ERR(m_symbolTable->Insert(ident, nodeType, false, bound));
                }

                if (currToken->type == T_RSQBRACKET)
                {
                    NEXT_TOKEN;
                    return ERROR_NONE;
                }
                return ERROR_MISSING_BRACKET;
            }

            // Insert into symbol table
            if (hasGlobal)
            {
                RET_IF_ERR(m_symbolTable->InsertGlobal(ident, nodeType, false, 0));
            }
            else
            {
                RET_IF_ERR(m_symbolTable->Insert(ident, nodeType, false, 0));
            }

            return ERROR_NONE;
        }
        return ERROR_MISSING_COLON;
    }

    return required ? ERROR_INVALID_VARIABLE_DECLARATION : ERROR_NO_OCCURRENCE;
}

ERROR_TYPE Parser::ProcedureHeader(TokenPR currToken, ParseNodePR nodeOut, bool required, bool hasGlobal)
{
    ERROR_TYPE error = ERROR_NONE;
    nodeOut = std::make_shared<ParseNode>();
    nodeOut->type = NodeType::PROCEDURE_HEADER;

    if (IS_CERTAIN_WORD(currToken, "procedure"))
    {
        NEXT_TOKEN;

        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Identifier(currToken, nextNode, true));

        // Save ident for later
        std::string ident = std::get<std::string>(nextNode->token->value);

        if (currToken->type == T_COLON)
        {
            NEXT_TOKEN;
            ParseNodeP nextNode = nullptr;
            REQ_PARSE(TypeMark(currToken, nextNode, true));

            ValueType procedureReturnType = nextNode->valueType;

            if (currToken->type == T_LPAREN)
            {
                NEXT_TOKEN;

                // Create new level of scope now for parameters and everything else
                RET_IF_ERR(m_symbolTable->AddLevel());

                nextNode = nullptr;
                TRY_PARSE(ParameterList(currToken, nextNode)); // Optional

                std::vector<ValueType> paramTypes = std::vector<ValueType>();
                if (nextNode) // Parameter List
                {
                    for (ParseNodeP param : nextNode->children)
                    {
                        paramTypes.push_back(param->valueType);
                    }
                }

                // Add procedure name to symbol table
                if (hasGlobal)
                {
                    RET_IF_ERR(m_symbolTable->InsertGlobal(ident, procedureReturnType, true, 0, paramTypes));
                }
                else
                {
                    RET_IF_ERR(m_symbolTable->InsertUp(ident, procedureReturnType, true, 0, paramTypes));
                }

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

        if (IS_CERTAIN_WORD(currToken, "integer"))
        {
            nodeOut->valueType = ValueType::INT;
        }
        else if (IS_CERTAIN_WORD(currToken, "float"))
        {
            nodeOut->valueType = ValueType::DOUBLE;
        }
        else if (IS_CERTAIN_WORD(currToken, "string"))
        {
            nodeOut->valueType = ValueType::STRING;
        }
        else if (IS_CERTAIN_WORD(currToken, "bool"))
        {
            nodeOut->valueType = ValueType::BOOL;
        }

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
    nodeOut->valueType = nextNode->valueType; // Pass VariableDeclaration type up

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

    ValueType destType = nextNode->valueType;

    if (currToken->type == T_ASSIGN)
    {
        NEXT_TOKEN;

        nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, required));

        ValueType expType = nextNode->valueType;

        if (destType == expType);
        else if (destType == ValueType::BOOL && expType == ValueType::INT);
        else if (destType == ValueType::INT && expType == ValueType::BOOL);
        else if (destType == ValueType::DOUBLE && expType == ValueType::INT);
        else if (destType == ValueType::INT && expType == ValueType::DOUBLE);
        else // If none of these, can't assign
        {
            return ERROR_MISMATCHED_TYPES;
        }

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

    
    Symbol *destinationSymbol = nullptr;
    m_symbolTable->Lookup(std::get<std::string>(nextNode->token->value), destinationSymbol);

    if (!destinationSymbol)
    {
        return ERROR_SYMBOL_DOESNT_EXIST;
    }

    nodeOut->valueType = destinationSymbol->type;

    if (currToken->type == T_LSQBRACKET)
    {
        NEXT_TOKEN;

        if (destinationSymbol->type < ValueType::INTARRAY)
        {
            return ERROR_SYMBOL_NOT_ARRAY;
        }

        nodeOut->valueType = (ValueType)((int)destinationSymbol->type - NOT_TO_ARRAY);

        nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, true));

        if (nextNode->valueType != ValueType::INT)
        {
            return ERROR_EXPECTED_INT;
        }

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

    ValueType valueType = ValueType::NOTHING;

    do
    {
        ParseNodeP nextNode = nullptr;
        REQ_PARSE(Factor(currToken, nextNode, requiredThisPass));
        done = true;

        if (valueType == ValueType::NOTHING)
        {
            valueType = nextNode->valueType;
        }
        else
        {
            if (nextNode->valueType == ValueType::STRING)
            {
                return ERROR_INVALID_OPERAND;
            }

            if (nextNode->valueType > valueType) // BOOL --> INT --> DOUBLE, stay at 'highest' one
            {
                valueType = nextNode->valueType;
            }
        }

        if (currToken->type == T_MULTIPLY || currToken->type == T_DIVIDE)
        {
            requiredThisPass = true;
            done = false;

            if (valueType == ValueType::STRING)
            {
                return ERROR_INVALID_OPERAND; // cannot multiply/divide strings
            }

            ParseNodeP opNode = std::make_shared<ParseNode>();
            opNode->type = NodeType::SYMBOL;
            opNode->token = currToken;
            nodeOut->children.push_back(opNode);

            NEXT_TOKEN;
        }
    } while (!done);

    nodeOut->valueType = valueType;

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

        nodeOut->valueType = nextNode->valueType;

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
        currToken->type = T_BOOLCONST;

        ParseNodeP trueNode = std::make_shared<ParseNode>();
        trueNode->type = NodeType::SYMBOL;
        trueNode->token = currToken;
        nodeOut->children.push_back(trueNode);
        nodeOut->valueType = ValueType::BOOL;


        NEXT_TOKEN;

        return ERROR_NONE;
    }

    if (IS_CERTAIN_WORD(currToken, "false"))
    {
        currToken->value = 0;
        currToken->type = T_BOOLCONST;

        ParseNodeP falseNode = std::make_shared<ParseNode>();
        falseNode->type = NodeType::SYMBOL;
        falseNode->token = currToken;
        nodeOut->children.push_back(falseNode);
        nodeOut->valueType = ValueType::BOOL;

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
            nodeOut->valueType = nextNode->valueType;
            return ERROR_NONE;
        }

        nodeOut->valueType = nextNode->valueType;

        return ERROR_NONE;
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
            nodeOut->valueType = ValueType::STRING;
            return ERROR_NONE;
        }
        nodeOut->valueType = nextNode->valueType;
        return ERROR_NONE;
    }
    nodeOut->valueType = nextNode->valueType;
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

        Symbol *procedureSymbol = nullptr;
        m_symbolTable->Lookup(std::get<std::string>(nextNode->token->value), procedureSymbol);

        if (!procedureSymbol)
        {
            return ERROR_SYMBOL_DOESNT_EXIST;
        }

        nodeOut->valueType = procedureSymbol->type;

        nextNode = nullptr;
        TRY_PARSE(ArgumentList(currToken, nextNode)); // Optional

        if (nextNode) // Check arguments
        {
            if (nextNode->children.size() != procedureSymbol->functionParameterTypes.size())
            {
                return ERROR_ARGUMENTS_DONT_MATCH;
            }

            for (int i = 0; i < nextNode->children.size(); ++i)
            {
                if (nextNode->children[i]->valueType != procedureSymbol->functionParameterTypes[i])
                {
                    return ERROR_ARGUMENTS_DONT_MATCH;
                }
            }
        }

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

        Symbol *nameSymbol = nullptr;
        m_symbolTable->Lookup(std::get<std::string>(nextNode->token->value), nameSymbol);

        if (!nameSymbol)
        {
            return ERROR_SYMBOL_DOESNT_EXIST;
        }

        nodeOut->valueType = nameSymbol->type;

        if (currToken->type == T_LSQBRACKET)
        {
            NEXT_TOKEN;

            if (nameSymbol->type < ValueType::INTARRAY) // Make sure it's an array if there's bounds
            {
                return ERROR_SYMBOL_NOT_ARRAY;
            }

            nodeOut->valueType = (ValueType)((int)nameSymbol->type - NOT_TO_ARRAY); // Go from array to not array

            nextNode = nullptr;
            REQ_PARSE(Expression(currToken, nextNode, true));

            if (nextNode->valueType != ValueType::INT)
            {
                return ERROR_EXPECTED_INT;
            }

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

    Symbol *nameSymbol = nullptr;
    m_symbolTable->Lookup(std::get<std::string>(nextNode->token->value), nameSymbol);

    if (!nameSymbol)
    {
        return ERROR_SYMBOL_DOESNT_EXIST;
    }

    nodeOut->valueType = nameSymbol->type;

    if (currToken->type == T_LSQBRACKET)
    {
        NEXT_TOKEN;

        if (nameSymbol->type < ValueType::INTARRAY)
        {
            return ERROR_SYMBOL_NOT_ARRAY;
        }

        nodeOut->valueType = (ValueType)((int)nameSymbol->type - NOT_TO_ARRAY);

        nextNode = nullptr;
        REQ_PARSE(Expression(currToken, nextNode, true));

        if (nextNode->valueType != ValueType::INT)
        {
            return ERROR_EXPECTED_INT;
        }

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

        if (currToken->type == T_INTCONST)
        {
            nodeOut->valueType = ValueType::INT;
        }
        else
        {
            nodeOut->valueType = ValueType::DOUBLE;
        }

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