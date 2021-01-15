#include <iostream>
#include <string>

#include "References.h"
#include "FileIn.h"

int main(int argc, char* args[])
{
    std::cout << "Hello Compiler\n";

    int error = ERROR_NONE;
    FileIn* inFile;

    if (argc == 2)
    {
        inFile = new FileIn(args[1]);
        error = inFile->OpenFile();
        
        if (error == ERROR_NONE)
        {
            std::cout << "OPENED SUCCESSFUL";
            inFile->CloseFile();
        }

        delete inFile;
    }

    return error;
}