#include <iostream>
#include <string>

#include "Utilities/Error.h"
#include "Utilities/FileIn.h"

int main(int argc, char* args[])
{
    std::cout << "Hello Compiler\n";

    int error = ERROR_NONE;
    FileIn* inFile;

    if (argc == 2)
    {
        inFile = new FileIn(args[1]);

        RET_IF_ERR(inFile->LoadFile());

        char c;
        int currLine, currChar;
        while (inFile->GetNextChar(c, currLine, currChar) != ERROR_END_OF_FILE)
        {
            std::cout << c;
        }

        delete inFile;
    }

    return error;
}