#ifndef _INCL_UTILITIES_FILE_IN
#define _INCL_UTILITIES_FILE_IN

#include <string>
#include <fstream>

#include "Utilities/Error.h"

class FileIn
{
public:
    FileIn();
    FileIn(std::string inFileName);

    void SetFileName(std::string inFileName);
    std::string GetFileName();

    ERROR_TYPE LoadFile(std::string inFileName = "");

    ERROR_TYPE GetNextChar(char &c, int &currLine, int &currLineChar);
    ERROR_TYPE PeekNextChar(char &c);

private:
    std::string m_inFileName;
    std::string m_fileString;

    int m_maxChar;
    int m_currChar;

    int m_currLine;
    int m_currLineChar;
};

#endif