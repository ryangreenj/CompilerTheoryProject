#ifndef _INCL_SYMBOL_TABLE
#define _INCL_SYMBOL_TABLE

#include <string>
#include <variant>
#include <vector>

#include "Utilities/Error.h"

enum class ValueType
{
    NOTHING = 0,
    BOOL,
    INT,
    DOUBLE,
    STRING,
    BOOLARRAY,
    INTARRAY,
    DOUBLEARRAY,
    STRINGARRAY,
};

#define NOT_TO_ARRAY ((int)ValueType::BOOLARRAY - (int)ValueType::BOOL)

struct Symbol
{
    std::string identifier;
    ValueType type;
    bool isFunction = false;
    std::variant<std::string, int, double> value = 0;
    std::vector<ValueType> functionParameterTypes;
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

    ERROR_TYPE Insert(std::string identifier, ValueType type, bool isFunction, std::variant<std::string, int, double> value, std::vector<ValueType> functionParameterTypes = std::vector<ValueType>());
    ERROR_TYPE InsertUp(std::string identifier, ValueType type, bool isFunction, std::variant<std::string, int, double> value, std::vector<ValueType> functionParameterTypes = std::vector<ValueType>());
    ERROR_TYPE InsertGlobal(std::string identifier, ValueType type, bool isFunction, std::variant<std::string, int, double> value, std::vector<ValueType> functionParameterTypes = std::vector<ValueType>());
    ERROR_TYPE Lookup(std::string identifier, Symbol *&symbolOut, bool checkGlobal = true);
    ERROR_TYPE LookupGlobal(std::string identifier, Symbol *&symbolOut);
    ERROR_TYPE Remove(std::string identifier);
    ERROR_TYPE RemoveGlobal(std::string identifier);

    ERROR_TYPE AddLevel();
    ERROR_TYPE DeleteLevel();

private:
    TableNode *m_head;
    TableNode *m_global;

    //int Hash(std::string identifier);
};

#endif