#ifndef _INCL_PARSE_TREE
#define _INCL_PARSE_TREE

#include <memory>
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
    PROCEDURE_HEADER,
    PROCEDURE_BODY,
    PARAMETER_LIST,
    PARAMETER,
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
    std::vector<std::shared_ptr<ParseNode>> children;
};

typedef std::shared_ptr<ParseNode> ParseNodeP;
typedef ParseNodeP& ParseNodePR;

#endif