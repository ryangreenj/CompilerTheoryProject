#include "Parser/ParseTree.h"

void ParseTree::PrintTree(std::ostream &out, ParseNodeP root, int layers)
{
    for (int i = 0; i < layers; ++i)
    {
        out << "  ";
    }

    out << NODE_TYPE_NAMES[(int)root->type] << '\n';
    if (root->token)
    {
        for (int i = 0; i < layers; ++i)
        {
            out << "  ";
        }
        out << "  TOKEN TYPE: " << root->token->type;

        switch (root->token->type)
        {
        case T_BOOLCONST:
        case T_INTCONST:
            out << "  TOKEN VALUE: " << std::get<int>(root->token->value);
            break;
        case T_DOUBLECONST:
            out << "  TOKEN VALUE: " << std::get<double>(root->token->value);
            break;
        case T_STRINGCONST:
        case T_IDENTIFIER:
            out << "  TOKEN VALUE: " << std::get<std::string>(root->token->value);
            break;
        default:
            break;
        }

        out << '\n';
    }

    for (ParseNodeP child : root->children)
    {
        PrintTree(out, child, layers + 1);
    }
}