#include "../parse/parser.h"
#include "../utils.h"
#include <stdio.h>

#define MAX_CODE_LENGTH 16384

SymbolTable* symbol_table;

int main(int argc, char** argv)
{
    symbol_table = new SymbolTable();
    // symbol_table->add_entry(SymbolTableEntry(std::string("a"), BASIC_TYPE_CONST_INT, 2));
    // symbol_table->add_entry(SymbolTableEntry(std::string("b"), BASIC_TYPE_INT, 3));
    // symbol_table->add_entry(SymbolTableEntry(std::string("c"), BASIC_TYPE_FUNC));
    // auto entry4 = SymbolTableEntry(std::string("d"), BASIC_TYPE_CONST_INT, {1, 2, 3, 4});
    // entry4.type.create_array_type(2);
    // entry4.type.create_array_type(2);
    // symbol_table->add_entry(entry4);
    // symbol_table->add_entry(SymbolTableEntry(std::string("e"), BASIC_TYPE_CONST_INT, 0));
    // symbol_table->add_entry(SymbolTableEntry(std::string("f"), BASIC_TYPE_INT, 1));

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[1], code, MAX_CODE_LENGTH);

    Parser parser(code);
    // symbol_table = symbol_table->create_subscope();
    while (true)
    {
        int decl = parser.parse_next_var_decl();
        if (decl == 0)
            break;
    }    
    symbol_table->print_table();
    printf("Hello\n");

    return 0;
}