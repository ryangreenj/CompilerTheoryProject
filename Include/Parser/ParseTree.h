#ifndef _INCL_PARSE_TREE
#define _INCL_PARSE_TREE

#include <vector>

#include "Utilities/Token.h"

enum class NodeType
{
    UNKNOWN,
    SYMBOL,
    PROGRAM,
    PROGRAM_HEADER,
    PROGRAM_BODY,
    DECLARATION,
    PROCEDURE_DECLARATION,
    PROCEDURE_BODY,
    VARIABLE_DECLARATION,
    TYPE_DECLARATION,
    TYPE_MARK,
    BOUND,
    STATEMENT,
    PROCEDURE_CALL,
    ASSIGNMENT_STATEMENT,
    DESTINATION,
    IF_STATEMENT,
    LOOP_STATEMENT,
    RETURN_STATEMENT,
    IDENTIFIER,
    EXPRESSION,
    ARITH_OP,
    RELATION,
    TERM,
    FACTOR,
    NAME,
    ARGUMENT_LIST,
    NUMBER,
    STRING
};

struct ParseNode
{
    NodeType type = NodeType::UNKNOWN;
    Token *token = nullptr;
    std::vector<ParseNode*> children;
};

typedef ParseNode* ParseNodeP;
typedef ParseNode*& ParseNodePR;

#endif
