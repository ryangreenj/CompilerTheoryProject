#include "Utilities/Error.h"

using namespace Error;

#include <ostream>
#include <sstream>
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

void Error::ReportError(ERROR_TYPE error, TokenP token)
{
    std::stringstream ss;

    if (token)
        ss << "Error at Line " << token->line << " Char " << token->startChar << ": " << ERROR_MESSAGES[error];
    else
        ss << "Error: " << ERROR_MESSAGES[error];
    ErrorList.push_back(ErrorTuple(error, ss.str()));
}

void Error::ReportWarning(ERROR_TYPE error, TokenP token)
{
    std::stringstream ss;

    if (token)
        ss << "Warning at Line " << token->line << " Char " << token->startChar << ": " << ERROR_MESSAGES[error];
    else
        ss << "Warning: " << ERROR_MESSAGES[error];
    WarningList.push_back(ErrorTuple(error, ss.str()));
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

void Error::PrintAllErrors(std::ostream &outStream)
{
    for (ErrorTuple e : ErrorList)
    {
        outStream << "ERROR: [" << std::get<0>(e) << "] " << std::get<1>(e) << "\n";
    }
}

void Error::PrintAllWarnings(std::ostream &outStream)
{
    for (ErrorTuple w : WarningList)
    {
        outStream << "WARNING: [" << std::get<0>(w) << "] " << std::get<1>(w) << "\n";
    }
}