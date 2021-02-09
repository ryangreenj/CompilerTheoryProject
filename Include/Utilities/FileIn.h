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

    ERROR_TYPE AdvanceChar();
    ERROR_TYPE AdvanceChar(int &currLine, int &currLineChar);
    ERROR_TYPE GetChar(char &c);
    ERROR_TYPE GetChar(char &c, int &currLine, int &currLineChar);
    ERROR_TYPE PeekChar(char &c, int ahead = 0);

private:
    std::string m_inFileName;
    std::string m_fileString;

    int m_maxChar;
    int m_currChar;

    int m_currLine;
    int m_currLineChar;
};

#endif