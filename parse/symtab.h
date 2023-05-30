#ifndef SYMTAB_H
#define SYMTAB_H

#include <vector>
#include <string>
#include <cstddef>

#define BASIC_TYPE_INT 0
#define BASIC_TYPE_CONST_INT 1
#define BASIC_TYPE_FUNC 2

struct Type
{
    int basic_type;
    std::vector<std::size_t> array_lengths;

    Type();
    ~Type();
    Type(int _basic_type);
    void create_array_type(std::size_t len);
    std::string to_str();
};

struct SymbolTableEntry
{
    std::string name;
    Type type;
    std::vector<int> init_val;
    std::vector<void*> init_exp;
    std::size_t addr;


    SymbolTableEntry() = default;
    SymbolTableEntry(const std::string& _name, int basic_type);
    /* Initialize by value (for constants) */
    SymbolTableEntry(const std::string& _name, int basic_type, int _init_val);
    SymbolTableEntry(const std::string& _name, int basic_type, const std::vector<int>& _init_val);
    /* Initialize by expression (for variables) */
    SymbolTableEntry(const std::string& _name, int basic_type, void* _init_exp);
    SymbolTableEntry(const std::string& _name, int basic_type, const std::vector<void*>& _init_exp);

    void set_init_val(int _init_val);
    void set_init_val(const std::vector<int>& _init_val);
    void set_init_exp(void* _init_exp);
    void set_init_exp(const std::vector<void*>& _init_exp);
    ~SymbolTableEntry() = default;
};

class SymbolTable
{
private:
    SymbolTable* parent;
    std::vector<SymbolTable*> subscopes;
    std::vector<SymbolTableEntry*> entries;
public:
    SymbolTable();
    ~SymbolTable();

    /**
     *  Copy and move are forbidden.
     */
    SymbolTable(const SymbolTable& _t) = delete;
    SymbolTable(SymbolTable&& _t) = delete;
    SymbolTable& operator=(const SymbolTable& _t) = delete;
    SymbolTable& operator=(SymbolTable&& _t) = delete;

    SymbolTable* parent_scope();
    SymbolTable* create_subscope();
    void add_entry(const SymbolTableEntry& entry);
    void delete_entry();
    void print_table();
    SymbolTableEntry* get_entry_if_contains(const char* _name);
    SymbolTableEntry* get_entry_if_contains_in_tree(const char* _name);
};


#endif