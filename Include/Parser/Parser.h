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

    ERROR_TYPE Program(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE ProgramHeader(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE ProgramBody(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE Declaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE Statement(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE
    ERROR_TYPE ProcedureDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE VariableDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE TypeDeclaration(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE
    ERROR_TYPE ProcedureHeader(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE ProcedureBody(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE TypeMark(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE ParameterList(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE Parameter(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE TESTED
    ERROR_TYPE Bound(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE
    ERROR_TYPE AssignmentStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE
    ERROR_TYPE IfStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE
    ERROR_TYPE LoopStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false); // DONE
    ERROR_TYPE ReturnStatement(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
    ERROR_TYPE ProcedureCall(TokenPR currToken, ParseNodePR nodeOut, bool required = false);
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
};

#endif