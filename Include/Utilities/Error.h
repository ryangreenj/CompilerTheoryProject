#ifndef _INCL_UTILITIES_ERROR
#define _INCL_UTILITIES_ERROR

typedef unsigned int ERROR_TYPE;

#define RET_IF_ERR(error) if (error != ERROR_NONE) return error;
#define RET_IF_NO_ERR(error) if (error == ERROR_NONE) return error;

#define ERROR_NONE 0
#define ERROR_FAIL_TO_OPEN 1
#define ERROR_END_OF_FILE 2
#define ERROR_UNEXPECTED_CHARACTER 3

#define ERROR_NO_PROGRAM_END 100
#define ERROR_NO_OCCURRENCE 101
#define ERROR_INVALID_HEADER 102
#define ERROR_INVALID_BODY 103
#define ERROR_INVALID_DECLARATION 104
#define ERROR_INVALID_TYPE_MARK 105
#define ERROR_INVALID_PARAMETER_LIST 106
#define ERROR_INVALID_ENUM 107
#define ERROR_INVALID_BOUND 108
#define ERROR_INVALID_TYPE_DECLARATION 109
#define ERROR_INVALID_VARIABLE_DECLARATION 110

#define ERROR_MISSING_IDENTIFIER 1000
#define ERROR_MISSING_SEMICOLON 1001
#define ERROR_MISSING_COLON 1002
#define ERROR_MISSING_BRACKET 1003
#define ERROR_MISSING_PAREN 1004

#include <string>

namespace Error
{
    void ReportError(ERROR_TYPE error, std::string message);
    void ReportWarning(ERROR_TYPE error, std::string message);

    bool HasError();
    bool HasWarning();

    void ClearAllErrors();
    void ClearAllWarnings();

    void PrintAllErrors(std::ostream outStream);
    void PrintAllWarnings(std::ostream outStream);
}

#endif