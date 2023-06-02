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

    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[1], code, MAX_CODE_LENGTH);

    Parser parser(code);
    auto exp = parser.parse_next_block();
    if (exp == nullptr)
    {
        fprintf(stderr, "Parsing failed!\n");
        delete symbol_table;
        exit(2);
    }
    IntermediateCodeGenerator gen;
    gen.generate_code_for_block_and_statement(exp);
    for (auto& c: gen.code)
    {
        printf("%s\n", c->to_str().c_str());
    }
    delete symbol_table;
    return 0;
}