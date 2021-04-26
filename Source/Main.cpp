#include <iostream>
#include <string>

#include "CodeGen/CodeGen.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Utilities/Error.h"
#include "Utilities/FileIn.h"
#include "Utilities/Token.h"

#include "llvm/IR/LLVMContext.h"

int main(int argc, char* args[])
{
    llvm::LLVMContext context;
    int error = ERROR_NONE;

    if (argc == 3)
    {
        Lexer *l = new Lexer(args[1]);
        Parser *p = new Parser(l);
        ParseNodeP tree = p->Parse();

        Error::PrintAllWarnings(std::cout);

        if (tree)
        {
            //ParseTree::PrintTree(std::cout, tree);
            CodeGen::Out(args[2]);
        }
        else
        {
            Error::PrintAllErrors(std::cout);
        }
    }
    else
    {
        std::cout << "Invalid arguments, should be: ./Compiler [InFileName] [OutFileName]\n";
    }

    return error;
}