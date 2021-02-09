#include <iostream>
#include <string>

#include "Utilities/Error.h"
#include "Utilities/FileIn.h"
#include "Utilities/Token.h"
#include "Lexer/Lexer.h"

int main(int argc, char* args[])
{
    std::cout << "Hello Compiler\n";

    int error = ERROR_NONE;
    FileIn* inFile;

    if (argc == 2)
    {
        Token *t = new Token();

        Lexer l(args[1]);
        while (t->type != T_EOF)
        {
            l.GetNextToken(t);
        }

        std::cout << "Done" << std::get<std::string>(t->value) << std::endl;
    }

    return error;
}