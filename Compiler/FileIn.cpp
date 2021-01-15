#include "FileIn.h"

FileIn::FileIn() : FileIn("") {}

FileIn::FileIn(std::string inFileName)
{
    m_inFileName = inFileName;
    m_inFile = std::ifstream(m_inFileName);
}

void FileIn::SetFileName(std::string inFileName)
{
    m_inFileName = inFileName;
}

std::string FileIn::GetFileName()
{
    return m_inFileName;
}

ERROR_TYPE FileIn::OpenFile()
{
    m_inFile.open(m_inFileName);
    return IsOpen() ? ERROR_NONE : ERROR_FAIL_TO_OPEN;
}

ERROR_TYPE FileIn::CloseFile()
{
    m_inFile.close();
    return ERROR_NONE;
}

bool FileIn::IsOpen()
{
    return m_inFile.is_open();
}

ERROR_TYPE FileIn::GetNextChar(char &c)
{
    return m_inFile.get(c) ? ERROR_NONE : ERROR_END_OF_FILE;
}