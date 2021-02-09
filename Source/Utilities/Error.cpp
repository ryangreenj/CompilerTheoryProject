#include "Utilities/Error.h"

#include <tuple>
#include <vector>

typedef std::tuple<ERROR_TYPE, std::string> ErrorTuple;

std::vector<ErrorTuple> ErrorList;
std::vector<ErrorTuple> WarningList;

void Error::ReportError(ERROR_TYPE error, std::string message)
{
    ErrorList.push_back(ErrorTuple(error, message));
}

void Error::ReportWarning(ERROR_TYPE error, std::string message)
{
    WarningList.push_back(ErrorTuple(error, message));
}

bool Error::HasError()
{
    return ErrorList.size();
}

bool Error::HasWarning()
{
    return WarningList.size();
}