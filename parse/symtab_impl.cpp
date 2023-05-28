#include "symtab.h"
#include <iostream>
#include <string.h>

SymbolTable::SymbolTable()
{
    this->parent = nullptr;
}

SymbolTable* SymbolTable::parent_scope()
{
    return this->parent;
}

SymbolTable* SymbolTable::create_subscope()
{
    SymbolTable* new_subscope = new SymbolTable();
    this->subscopes.push_back(new_subscope);

    new_subscope->parent = this;
    return new_subscope;
}

void SymbolTable::add_entry(const SymbolTableEntry& entry)
{
    this->entries.push_back(entry);
}

SymbolTable::~SymbolTable()
{
    for (auto& subscope : this->subscopes)
    {
        delete subscope;
    }
}

/**
 *  Return pointer to the symbol table entry if "_name" is in the symbol table;
 *  otherwise, return nullptr.
 */
SymbolTableEntry* SymbolTable::get_entry_if_contains(const char* _name)
{
    for (auto& entry: this->entries)
    {
        if (!strcmp(_name, entry.name.c_str()))
        {
            return &entry;
        }
    }
    return nullptr;
}

/**
 *  Return pointer to the symbol table entry if "_name" is in the symbol table tree;
 *  otherwise, return nullptr.
 */
SymbolTableEntry* SymbolTable::get_entry_if_contains_in_tree(const char* _name)
{
    auto search_table = this;
    while (search_table != nullptr)
    {
        auto entry = search_table->get_entry_if_contains(_name);
        if (entry != nullptr)
            return entry;
        search_table = search_table->parent_scope();
    }
    return nullptr;
}