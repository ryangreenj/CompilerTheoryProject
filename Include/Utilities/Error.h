#ifndef _INCL_UTILITIES_ERROR
#define _INCL_UTILITIES_ERROR

#define RET_IF_ERR(error) if (error != ERROR_NONE) return error;
#define RET_IF_NO_ERR(error) if (error == ERROR_NONE) return error;

enum ERROR_TYPE
{
    ERROR_NONE = 0,
    ERROR_FAIL_TO_OPEN,
    ERROR_END_OF_FILE,
    ERROR_UNEXPECTED_CHARACTER,

    ERROR_NO_PROGRAM_END,
    ERROR_NO_OCCURRENCE,
    ERROR_INVALID_PROGRAM_HEADER,
    ERROR_INVALID_PROGRAM_BODY,
    ERROR_INVALID_DECLARATION,
    ERROR_INVALID_TYPE_MARK,
    ERROR_INVALID_PARAMETER_LIST,
    ERROR_INVALID_ENUM,
    ERROR_INVALID_BOUND,
    ERROR_INVALID_TYPE_DECLARATION,
    ERROR_INVALID_VARIABLE_DECLARATION,
    ERROR_INVALID_STATEMENT,
    ERROR_INVALID_IF_STATEMENT,
    ERROR_INVALID_LOOP_STATEMENT,
    ERROR_INVALID_RETURN_STATEMENT,
    ERROR_INVALID_IDENTIFIER,
    ERROR_INVALID_PROCEDURE_CALL_OR_NAME,
    ERROR_INVALID_STRING,
    ERROR_INVALID_NUMBER,
    ERROR_INVALID_DESTINATION,
    ERROR_INVALID_ARGUMENT_LIST,
    ERROR_INVALID_PROCEDURE_HEADER,
    ERROR_INVALID_PROCEDURE_BODY,
    ERROR_INVALID_PROCEDURE_DECLARATION,
    ERROR_INVALID_EXPRESSION,
    ERROR_INVALID_ARITH_OP,
    ERROR_INVALID_RELATION,
    ERROR_INVALID_TERM,
    ERROR_INVALID_FACTOR,

    ERROR_MISSING_SEMICOLON,
    ERROR_MISSING_COLON,
    ERROR_MISSING_BRACKET,
    ERROR_MISSING_PAREN,
    ERROR_MISSING_ASSIGN,

    ERROR_NO_TABLE,
    ERROR_SYMBOL_DOESNT_EXIST,
    ERROR_SYMBOL_ALREADY_EXISTS,

    ERROR_SYMBOL_NOT_ARRAY,
    ERROR_EXPECTED_INT,
    ERROR_INVALID_OPERAND,
    ERROR_MISMATCHED_TYPES,
    ERROR_ARGUMENTS_DONT_MATCH,
};

#include <string>

#include "Utilities/Token.h"

namespace Error
{
    void ReportError(ERROR_TYPE error, std::string message);
    void ReportWarning(ERROR_TYPE error, std::string message);

    void ReportError(ERROR_TYPE error, TokenP token = nullptr);
    void ReportWarning(ERROR_TYPE error, TokenP token = nullptr);

    bool HasError();
    bool HasWarning();

    void ClearAllErrors();
    void ClearAllWarnings();

    void PrintAllErrors(std::ostream &outStream);
    void PrintAllWarnings(std::ostream &outStream);

    const std::string ERROR_MESSAGES[] = {
        "No error",
        "Unable to open file",
        "Reached end of file",
        "Unexpected character found",

        "Program does not contain end sequence",
        "No occurrence",
        "Program header is invalid",
        "Program body is invalid",
        "Declaration is invalid",
        "Type mark is invalid",
        "Parameter list is invalid",
        "ENUM",
        "Bound is invalid",
        "TYPE DELCARATION",
        "Invalid variable declaration",
        "Invalid statement",
        "Invalid if statement",
        "Invalid loop statement",
        "Invalid return statement",
        "Invalid identifier",
        "Invalid procedure call or name",
        "Invalid string",
        "Invalid number",
        "Invalid destination",
        "Invalid argument list",
        "Invalid procedure header",
        "Invalid procedure body",
        "Invalid procedure declaration",
        "Invalid expression",
        "Invalid arithmetic operation",
        "Invalid relation",
        "Invalid term",
        "Invalid factor",

        "Missing semicolon",
        "Missing colon",
        "Missing bracket",
        "Missing parenthesis",
        "Missing assignment",
        
        "Symbol table does not exist",
        "Symbol does not exist in current scope",
        "Duplicate symbol declaration in current scope",

        "Symbol is not an array",
        "Expected integer",
        "Invalid operand type",
        "Mismatched types",
        "Arguments do not match procedure declaration"
    };
}

#endif