#ifndef _INCL_UTILITIES_ERROR
#define _INCL_UTILITIES_ERROR

typedef unsigned int ERROR_TYPE;

#define RET_IF_ERR(error) if (error != ERROR_NONE) return error;

#define ERROR_NONE 0
#define ERROR_FAIL_TO_OPEN 1
#define ERROR_END_OF_FILE 2

#include <string>

namespace Error
{
    void ReportError(ERROR_TYPE error, std::string message);
    void ReportWarning(ERROR_TYPE error, std::string message);
}

#endif