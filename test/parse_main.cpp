#include "../parse/parser.h"
#include "../utils.h"
#include <stdio.h>

#define MAX_CODE_LENGTH 16384

SymbolTable* symbol_table;

int main(int argc, char** argv)
{
    symbol_table = new SymbolTable();
    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[1], code, MAX_CODE_LENGTH);

    Parser parser(code);
    if (parser.parse())
    {
        symbol_table->print_table();
    }
    delete symbol_table;
    return 0;
}