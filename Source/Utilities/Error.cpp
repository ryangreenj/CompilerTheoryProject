#include "Utilities/Error.h"

using namespace Error;

#include <ostream>
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

void Error::ClearAllErrors()
{
    ErrorList.clear();
}

void Error::ClearAllWarnings()
{
    WarningList.clear();
}

void Error::PrintAllErrors(std::ostream outStream)
{
    for (ErrorTuple e : ErrorList)
    {
        outStream << "ERROR: [" << std::get<0>(e) << "] " << std::get<1>(e) << "\n";
    }
}

void Error::PrintAllWarnings(std::ostream outStream)
{
    for (ErrorTuple w : WarningList)
    {
        outStream << "WARNING: [" << std::get<0>(w) << "] " << std::get<1>(w) << "\n";
    }
}