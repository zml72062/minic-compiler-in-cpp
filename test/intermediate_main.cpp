#include "../intermediate/intermediate.h"
#include "../parse/parser.h"
#include "../utils.h"
#include <stdio.h>

#define MAX_CODE_LENGTH 16384

SymbolTable* symbol_table;
std::map<std::size_t, std::string> index_to_label;

int main(int argc, char** argv)
{    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    symbol_table = new SymbolTable();
    auto entry = SymbolTableEntry("a", BASIC_TYPE_CONST_INT);
    entry.type.create_array_type(2);
    entry.type.create_array_type(3);
    entry.set_init_val({1,2,3,6,5,4});
    symbol_table->add_entry(entry);
    auto entry2 = SymbolTableEntry("b", BASIC_TYPE_INT);
    entry2.type.create_array_type(5);
    entry2.type.create_array_type(3);
    symbol_table->add_entry(entry2);
    auto entry3 = SymbolTableEntry("c", BASIC_TYPE_FUNC);
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT, {0}));
    entry3.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    symbol_table->add_entry(entry3);


    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[1], code, MAX_CODE_LENGTH);

    Parser parser(code);
    auto exp = parser.parse_next_exp();
    if (exp == nullptr)
    {
        fprintf(stderr, "Parsing failed!\n");
        delete symbol_table;
        exit(2);
    }
    IntermediateCodeGenerator gen;
    gen.generate_code_for_exp(exp);
    for (auto& c: gen.code)
    {
        printf("%s\n", c->to_str().c_str());
    }
    delete symbol_table;
    return 0;
}