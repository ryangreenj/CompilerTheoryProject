#ifndef _INCL_PARSE_TREE
#define _INCL_PARSE_TREE

#include <memory>
#include <ostream>
#include <vector>

#include "Utilities/SymbolTable.h"
#include "Utilities/Token.h"

enum class NodeType
{
    UNKNOWN = 0,
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

const std::string NODE_TYPE_NAMES[] =
{
    "UNKNOWN",
    "SYMBOL",
    "PROGRAM",
    "PROGRAM_HEADER",
    "PROGRAM_BODY",
    "DECLARATION",
    "PROCEDURE_DECLARATION",
    "PROCEDURE_HEADER",
    "PROCEDURE_BODY",
    "PARAMETER_LIST",
    "PARAMETER",
    "VARIABLE_DECLARATION",
    "TYPE_DECLARATION",
    "TYPE_MARK",
    "BOUND",
    "STATEMENT",
    "PROCEDURE_CALL",
    "ASSIGNMENT_STATEMENT",
    "DESTINATION",
    "IF_STATEMENT",
    "LOOP_STATEMENT",
    "RETURN_STATEMENT",
    "IDENTIFIER",
    "EXPRESSION",
    "ARITH_OP",
    "RELATION",
    "TERM",
    "FACTOR",
    "NAME",
    "ARGUMENT_LIST",
    "NUMBER",
    "STRING"
};

struct ParseNode
{
    NodeType type = NodeType::UNKNOWN;
    TokenP token = nullptr;
    ValueType valueType = ValueType::NOTHING;
    std::vector<std::shared_ptr<ParseNode>> children;
};

typedef std::shared_ptr<ParseNode> ParseNodeP;
typedef ParseNodeP& ParseNodePR;

namespace ParseTree
{
    void PrintTree(std::ostream &out, ParseNodeP root, int layers = 0);
}

#endif
