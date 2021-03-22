#include "Utilities/SymbolTable.h"


// TableNode

TableNode::TableNode(TableNode *next)
{
    this->m_next = next;
}

TableNode::~TableNode()
{
    for (Symbol *s : m_symbols)
    {
        delete s;
    }
}

void TableNode::AddSymbol(Symbol *toAdd)
{
    m_symbols.push_back(toAdd);
}

Symbol* TableNode::GetSymbol(std::string identifier)
{
    for (Symbol *s : m_symbols)
    {
        if (s->identifier.compare(identifier) == 0)
        {
            return s;
        }
    }

    return nullptr;
}

bool TableNode::RemoveSymbol(std::string identifier)
{
    for (int i = 0; i < m_symbols.size(); ++i)
    {
        if (m_symbols[i]->identifier.compare(identifier) == 0)
        {
            delete m_symbols[i];
            m_symbols.erase(m_symbols.begin() + i);

            return true;
        }
    }

    return false;
}


// SymbolTable

SymbolTable::SymbolTable()
{
    m_head = nullptr;
    m_global = new TableNode();
}

ERROR_TYPE SymbolTable::Insert(std::string identifier, std::string type, std::variant<std::string, int, double> value)
{
    if (!m_head)
    {
        // Currently in global scope
        return InsertGlobal(identifier, type, value);
    }
    else
    {
        // First check if it exists in the table already
        Symbol *symbolFind = nullptr;
        if (Lookup(identifier, symbolFind, false) == ERROR_NONE)
        {
            if (symbolFind)
            {
                return ERROR_SYMBOL_ALREADY_EXISTS;
            }
        }

        // Add it
        Symbol *toAdd = new Symbol();
        toAdd->identifier = identifier;
        toAdd->type = type;
        toAdd->value = value;

        m_head->AddSymbol(toAdd);
        return ERROR_NONE;
    }
}

ERROR_TYPE SymbolTable::InsertGlobal(std::string identifier, std::string type, std::variant<std::string, int, double> value)
{
    // First check if it exists in the table already
    Symbol *symbolFind = nullptr;
    if (LookupGlobal(identifier, symbolFind) == ERROR_NONE)
    {
        if (symbolFind)
        {
            return ERROR_SYMBOL_ALREADY_EXISTS;
        }
    }

    // Add it
    Symbol *toAdd = new Symbol();
    toAdd->identifier = identifier;
    toAdd->type = type;
    toAdd->value = value;

    m_global->AddSymbol(toAdd);
    return ERROR_NONE;
}

ERROR_TYPE SymbolTable::Lookup(std::string identifier, Symbol *&symbolOut, bool checkGlobal)
{
    symbolOut = m_head->GetSymbol(identifier);

    if (checkGlobal)
    {
        return LookupGlobal(identifier, symbolOut);
    }

    return ERROR_NONE;
}

ERROR_TYPE SymbolTable::LookupGlobal(std::string identifier, Symbol *&symbolOut)
{
    symbolOut = m_global->GetSymbol(identifier);

    return ERROR_NONE;
}

ERROR_TYPE SymbolTable::Remove(std::string identifier)
{
    if (m_head) // Remove in current scope
    {
        return m_head->RemoveSymbol(identifier) ? ERROR_NONE : ERROR_SYMBOL_DOESNT_EXIST;
    }
    else
    {
        return RemoveGlobal(identifier);
    }
}

ERROR_TYPE SymbolTable::RemoveGlobal(std::string identifier)
{
    return m_global->RemoveSymbol(identifier) ? ERROR_NONE : ERROR_SYMBOL_DOESNT_EXIST;
}

ERROR_TYPE SymbolTable::AddLevel()
{
    m_head = new TableNode(m_head);
    
    return ERROR_NONE;
}

ERROR_TYPE SymbolTable::DeleteLevel()
{
    if (m_head)
    {
        TableNode *toDelete = m_head;
        m_head = m_head->m_next;
        delete toDelete;

        return ERROR_NONE;
    }
    return ERROR_NO_TABLE;
}