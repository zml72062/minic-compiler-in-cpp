#include "../ra_opt/basicblock.h"
#include "../intermediate/intermediate.h"
#include "../parse/parser.h"
#include "../utils.h"
#include <stdio.h>

#define MAX_CODE_LENGTH 16384

SymbolTable* symbol_table;

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
    if (!parser.parse())
    {
        fprintf(stderr, "Parsing failed!\n");
        delete symbol_table;
        exit(2);
    }
    IntermediateCodeGenerator gen;
    gen.generate_code();
    if (gen.error)
    {
        fprintf(stderr, "Intermediate code generation failed!\n");
        delete symbol_table;
        exit(3);
    }
    auto code_out = gen.simplify_code();
    auto proc = make_procedures(code_out);
    for (auto&  p: proc)
    {
        auto blocks = make_basic_blocks(p->code);

        LivenessUpdater updater(blocks);
        updater.calculate_liveness();

        RegisterAllocator alloc(AllocationTable(p->register_range()));
        alloc.allocate(updater);

        for (auto & b: blocks)
        {
            delete b;
        }
    }
    MemorySpiller().spill(code_out);
    remove_useless_mov(code_out);
    printf("\n");
    for (auto& line: code_out)
    {
        printf("%s\n", line->to_str().c_str());
    }
    for (auto& p: proc)
    {
        delete p;
    }
    for (auto& c: code_out)
    {
        delete c;
    }
    delete symbol_table;
    return 0;
}