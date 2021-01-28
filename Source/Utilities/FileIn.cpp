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

ERROR_TYPE FileIn::AdvanceChar(int &currLine, int &currLineChar)
{
    if (m_currChar >= m_maxChar)
    {
        return ERROR_END_OF_FILE;
    }

    if (m_fileString[m_currChar] == '\n')
    {
        ++m_currLine;
        m_currLineChar = 0;
    }

    ++m_currChar;

    currLine = m_currLine;
    currLineChar = m_currLineChar++;

    return ERROR_NONE;
}

ERROR_TYPE FileIn::GetChar(char &c, int &currLine, int &currLineChar)
{
    ERROR_TYPE error = ERROR_NONE;
    
    RET_IF_ERR(this->PeekChar(c));

    error = AdvanceChar(currLine, currLineChar);

    return error;
}

ERROR_TYPE FileIn::PeekChar(char &c, int ahead)
{
    if (m_currChar + ahead >= m_maxChar)
    {
        c = '\0';
        return ERROR_END_OF_FILE;
    }

    c = m_fileString[m_currChar + ahead];

    return ERROR_NONE;
}