#include <iostream>
#include <string>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Utilities/Error.h"
#include "Utilities/FileIn.h"
#include "Utilities/Token.h"

int main(int argc, char* args[])
{
    std::cout << "Hello Compiler\n";

    int error = ERROR_NONE;

    if (argc == 2)
    {
        Lexer *l = new Lexer(args[1]);
        Parser *p = new Parser(l);
        p->Parse();

        std::cout << "Done" << std::endl;
    }

    return error;
}