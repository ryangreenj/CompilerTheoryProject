#include "Utilities/FileIn.h"

FileIn::FileIn() : FileIn("") {}

FileIn::FileIn(std::string inFileName)
{
    m_inFileName = inFileName;
}

void FileIn::SetFileName(std::string inFileName)
{
    m_inFileName = inFileName;
}

std::string FileIn::GetFileName()
{
    return m_inFileName;
}

ERROR_TYPE FileIn::LoadFile(std::string inFileName)
{
    if (inFileName != "")
    {
        SetFileName(inFileName);
    }

    std::ifstream inFile(m_inFileName, std::ios::in | std::ios::binary);
    if (!inFile.is_open())
    {
        return ERROR_FAIL_TO_OPEN;
    }

    inFile.seekg(0, std::ios::end);
    m_fileString.resize(m_maxChar = inFile.tellg());
    inFile.seekg(0, std::ios::beg);
    inFile.read(&m_fileString[0], m_fileString.size());

    inFile.close();
    
    m_currChar = 0;
    m_currLine = 0;
    m_currLineChar = 0;

    return ERROR_NONE;
}

ERROR_TYPE FileIn::GetNextChar(char &c, int &currLine, int &currLineChar)
{
    ERROR_TYPE error = ERROR_NONE;
    
    RET_IF_ERR(this->PeekNextChar(c));

    ++m_currChar;

    currLine = m_currLine;
    currLineChar = m_currLineChar++;

    if (c == '\n')
    {
        ++currLine;
        currLineChar = 0;
    }

    return error;
}

ERROR_TYPE FileIn::PeekNextChar(char &c)
{
    if (m_currChar >= m_maxChar)
    {
        return ERROR_END_OF_FILE;
    }

    c = m_fileString[m_currChar];

    return ERROR_NONE;
}