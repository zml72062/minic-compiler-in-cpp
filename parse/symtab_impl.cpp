#include "symtab.h"
#include <iostream>
#include <string.h>
#include "../utils.h"
#include "symbols.h"

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
    this->entries.push_back(new SymbolTableEntry(entry));
}

void SymbolTable::delete_entry()
{
    delete this->entries.back();
    this->entries.pop_back();
}


SymbolTable::~SymbolTable()
{
    for (auto& subscope : this->subscopes)
    {
        delete subscope;
    }
    for (auto& entry : this->entries)
    {
        delete entry;
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
        if (!strcmp(_name, entry->name.c_str()))
        {
            return entry;
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

void SymbolTable::print_table()
{
    for (auto& entry: entries)
    {
        std::cout << entry->name << std::endl;
        std::cout << Number(entry->type.array_lengths, entry->init_val).to_str() << std::endl;
        auto size = entry->init_exp.size();
        std::vector<Symbol*> value_cast(size);
        for (auto i = 0; i < size; i++)
            value_cast[i] = (Symbol*)(entry->init_exp[i]);
        std::cout << ExpArray(entry->type.array_lengths, value_cast).to_str() << std::endl;
    }
}
