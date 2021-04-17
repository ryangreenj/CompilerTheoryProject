#include <iostream>
#include <string>

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Utilities/Error.h"
#include "Utilities/FileIn.h"
#include "Utilities/Token.h"

#include "llvm/IR/LLVMContext.h"

int main(int argc, char* args[])
{
    std::cout << "Hello Compiler\n";
    llvm::LLVMContext context;
    int error = ERROR_NONE;

    if (argc == 2)
    {
        Lexer *l = new Lexer(args[1]);
        Parser *p = new Parser(l);
        ParseNodeP tree = p->Parse();

        Error::PrintAllWarnings(std::cout);

        if (tree)
        {
            ParseTree::PrintTree(std::cout, tree);
        }
        else
        {
            Error::PrintAllErrors(std::cout);
        }
    }

    return error;
}