#include "../parse/symtab.h"

/**
 *  Delete all subscopes, keep only the global symbol table.
 */
void SymbolTable::to_global()
{
    for (auto& subscope : this->subscopes)
    {
        delete subscope;
    }
    this->subscopes.clear();
}