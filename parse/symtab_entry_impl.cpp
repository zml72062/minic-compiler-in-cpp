#include "symtab.h"

SymbolTableEntry::SymbolTableEntry(): name(), type(), init_val(), init_exp()
{
    this->func_def = nullptr;
    this->addr = 0;
}

SymbolTableEntry::SymbolTableEntry(const std::string& _name, int basic_type)
{
    this->name = _name;
    this->type.basic_type = basic_type;
    this->func_def = nullptr;
    this->addr = 0;
}

SymbolTableEntry::SymbolTableEntry(const std::string& _name, int basic_type, int _init_val)
{
    this->name = _name;
    this->type.basic_type = basic_type;
    this->init_val = std::vector<int>();
    this->init_val.push_back(_init_val);
    this->func_def = nullptr;
    this->addr = 0;
}

SymbolTableEntry::SymbolTableEntry(const std::string& _name, int basic_type, const std::vector<int>& _init_val)
{
    this->name = _name;
    this->type.basic_type = basic_type;
    this->init_val = _init_val;
    this->func_def = nullptr;
    this->addr = 0;
}

SymbolTableEntry::SymbolTableEntry(const std::string& _name, int basic_type, void* _init_exp)
{
    this->name = _name;
    this->type.basic_type = basic_type;
    this->init_exp = std::vector<void*>();
    this->init_exp.push_back(_init_exp);
    this->func_def = nullptr;
    this->addr = 0;
}

SymbolTableEntry::SymbolTableEntry(const std::string& _name, int basic_type, const std::vector<void*>& _init_exp)
{
    this->name = _name;
    this->type.basic_type = basic_type;
    this->init_exp = _init_exp;
    this->func_def = nullptr;
    this->addr = 0;
}

void SymbolTableEntry::set_init_val(int _init_val)
{
    this->init_val = std::vector<int>({_init_val});
}

void SymbolTableEntry::set_init_val(const std::vector<int>& _init_val)
{
    this->init_val = _init_val;
}

void SymbolTableEntry::set_init_exp(void* _init_exp)
{
    this->init_exp = std::vector<void*>({_init_exp});
}

void SymbolTableEntry::set_init_exp(const std::vector<void*>& _init_exp)
{
    this->init_exp = _init_exp;
}

void SymbolTableEntry::set_func_def(void* _func_def)
{
    this->func_def = _func_def;
}

SymbolTableEntry& SymbolTableEntry::global()
{
    this->is_global = 1;
    return *this;
}

SymbolTableEntry& SymbolTableEntry::local()
{
    this->is_global = 0;
    return *this;
}