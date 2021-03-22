#ifndef _INCL_SYMBOL_TABLE
#define _INCL_SYMBOL_TABLE

#include <string>
#include <variant>
#include <vector>

#include "Utilities/Error.h"

struct Symbol
{
    std::string identifier;
    std::string type;
    std::variant<std::string, int, double> value = 0;
};

class TableNode
{
public:
    TableNode() : TableNode(nullptr) {}
    TableNode(TableNode *next);
    ~TableNode();

    void AddSymbol(Symbol *toAdd);
    Symbol* GetSymbol(std::string identifier);
    bool RemoveSymbol(std::string identifier);

    TableNode *m_next;

private:
    std::vector<Symbol*> m_symbols;
};

class SymbolTable
{
public:
    SymbolTable();

    ERROR_TYPE Insert(std::string identifier, std::string type, std::variant<std::string, int, double> value);
    ERROR_TYPE InsertGlobal(std::string identifier, std::string type, std::variant<std::string, int, double> value);
    ERROR_TYPE Lookup(std::string identifier, Symbol *&symbolOut, bool checkGlobal = true);
    ERROR_TYPE LookupGlobal(std::string identifier, Symbol *&symbolOut);
    ERROR_TYPE Remove(std::string identifier);
    ERROR_TYPE RemoveGlobal(std::string identifier);

    ERROR_TYPE AddLevel();
    ERROR_TYPE DeleteLevel();

private:
    TableNode *m_head;
    TableNode *m_global;

    int Hash(std::string identifier);
};

#endif