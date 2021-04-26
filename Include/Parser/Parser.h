#ifndef _INCL_PARSER
#define _INCL_PARSER

#include "Lexer/Lexer.h"
#include "Parser/ParseTree.h"
#include "Utilities/Error.h"
#include "Utilities/SymbolTable.h"
#include "Utilities/Token.h"

class Parser
{
public:
    Parser(Lexer *lexerIn);
    ParseNodeP Parse();
private:
    Lexer *m_lexer;

    ERROR_TYPE Program(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ProgramHeader(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ProgramBody(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Declaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Statement(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ProcedureDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false, bool hasGlobal = false);
    ERROR_TYPE VariableDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false, bool hasGlobal = false);
    ERROR_TYPE VariableDeclarationNoInsert(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ProcedureHeader(TokenPR currToken, ParseNodePR nodeOut, bool required = false, bool hasGlobal = false);
    ERROR_TYPE ProcedureBody(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE TypeMark(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ParameterList(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Parameter(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Bound(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE AssignmentStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE IfStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE LoopStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ReturnStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ProcedureCallOrName(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // <procedure_call> and <name> both start with identifier...  
    ERROR_TYPE ArgumentList(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Destination(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Expression(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ArithOp(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Relation(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Term(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Factor(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Name(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Number(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE String(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE Identifier(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
};

#endif