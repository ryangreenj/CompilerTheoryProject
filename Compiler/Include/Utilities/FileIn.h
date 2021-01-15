#ifndef _INCL_UTILITIES_FILE_IN
#define _INCL_UTILITIES_FILE_IN

#include <string>
#include <fstream>

#include "References.h"

class FileIn
{
public:
    FileIn();
    FileIn(std::string inFileName);

    void SetFileName(std::string inFileName);
    std::string GetFileName();

    ERROR_TYPE OpenFile();
    ERROR_TYPE CloseFile();
    bool IsOpen();

    ERROR_TYPE GetNextChar(char &c);

private:
    std::string m_inFileName;
    std::ifstream m_inFile;
};

#endif