#include "../parse/parser.h"
#include "../utils.h"
#include <stdio.h>

#define MAX_CODE_LENGTH 16384

SymbolTable* symbol_table;

int main(int argc, char** argv)
{
    symbol_table = new SymbolTable();
    symbol_table->add_entry(SymbolTableEntry(std::string("a"), BASIC_TYPE_CONST_INT, 2));
    symbol_table->add_entry(SymbolTableEntry(std::string("b"), BASIC_TYPE_INT, 3));
    auto entry3 = SymbolTableEntry(std::string("c"), BASIC_TYPE_FUNC);
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT, {0, 2}));
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT, {0}));
    symbol_table->add_entry(entry3);
    auto entry4 = SymbolTableEntry(std::string("d"), BASIC_TYPE_CONST_INT, {1, 2, 3, 4});
    entry4.type.create_array_type(2);
    entry4.type.create_array_type(2);
    symbol_table->add_entry(entry4);
    symbol_table->add_entry(SymbolTableEntry(std::string("e"), BASIC_TYPE_CONST_INT, 0));
    symbol_table->add_entry(SymbolTableEntry(std::string("f"), BASIC_TYPE_INT, 1));

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[1], code, MAX_CODE_LENGTH);

    Parser parser(code);
    while (true)
    {
        Symbol* exp = parser.parse_next_exp();
        if (exp == nullptr)
            return 0;
        printf("%s\n", exp->to_str().c_str());
    }
    return 0;
}