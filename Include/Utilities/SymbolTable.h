#ifndef _INCL_SYMBOL_TABLE
#define _INCL_SYMBOL_TABLE

#include <string>
#include <variant>
#include <vector>

#include "Utilities/Error.h"

#include <llvm/IR/Instructions.h>
#include "llvm/IR/Value.h"

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
    llvm::AllocaInst *IRAllocaInst = nullptr;
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

    llvm::AllocaInst* GetReturnAllocaInst();
    void SetReturnAllocaInst(llvm::AllocaInst *al);

    TableNode *m_next;

private:
    std::vector<Symbol*> m_symbols;
    llvm::AllocaInst *m_returnAllocaInst;
};

class SymbolTable
{
public:
    //SymbolTable();

    static void InitSymbolTable();

    static ERROR_TYPE Insert(std::string identifier, ValueType type, bool isFunction, std::variant<std::string, int, double> value, std::vector<ValueType> functionParameterTypes = std::vector<ValueType>());
    static ERROR_TYPE InsertUp(std::string identifier, ValueType type, bool isFunction, std::variant<std::string, int, double> value, std::vector<ValueType> functionParameterTypes = std::vector<ValueType>());
    static ERROR_TYPE InsertGlobal(std::string identifier, ValueType type, bool isFunction, std::variant<std::string, int, double> value, std::vector<ValueType> functionParameterTypes = std::vector<ValueType>());
    static ERROR_TYPE Lookup(std::string identifier, Symbol *&symbolOut, bool checkGlobal = true);
    static ERROR_TYPE LookupGlobal(std::string identifier, Symbol *&symbolOut);
    static ERROR_TYPE Remove(std::string identifier);
    static ERROR_TYPE RemoveGlobal(std::string identifier);

    static llvm::AllocaInst *GetIRAllocaInst(std::string identifier);
    static void SetIRAllocaInst(std::string identifier, llvm::AllocaInst *IRAllocaInst);

    static llvm::AllocaInst *GetReturnAllocaInst();
    static void SetReturnAllocaInst(llvm::AllocaInst *al);

    static ERROR_TYPE AddLevel();
    static ERROR_TYPE DeleteLevel();

private:

    //int Hash(std::string identifier);
};

#endif