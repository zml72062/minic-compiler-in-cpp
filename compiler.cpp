#include "ra_opt/basicblock.h"
#include "intermediate/intermediate.h"
#include "parse/parser.h"
#include "utils.h"
#include "codegen/codegen.h"
#include <stdio.h>
#include <string.h>
#include <fstream>

#define MAX_CODE_LENGTH 1048576

SymbolTable* symbol_table;

int main(int argc, char** argv)
{    
    if (argc != 5 || strcmp(argv[1], "-riscv") || strcmp(argv[3], "-o"))
    {
        fprintf(stderr, "Usage: %s -riscv <in-file> -o <out-file>\n", argv[0]);
        exit(1);
    }

    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[2], code, MAX_CODE_LENGTH);

    std::size_t lib_start = (3 << 29);
    symbol_table = new SymbolTable();

    /* Add SysY built-in functions to symbol table. */
    SymbolTableEntry getint("getint", BASIC_TYPE_FUNC);
    getint.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    getint.addr = lib_start;
    SymbolTableEntry getch("getch", BASIC_TYPE_FUNC);
    getch.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    getch.addr = lib_start + 1;
    SymbolTableEntry getarray("getarray", BASIC_TYPE_FUNC);
    getarray.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    getarray.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT, {0}));
    getarray.addr = lib_start + 2;
    SymbolTableEntry putint("putint", BASIC_TYPE_FUNC);
    putint.type.add_ret_or_arg_type(Type(BASIC_TYPE_NONE));
    putint.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    putint.addr = lib_start + 3;
    SymbolTableEntry putch("putch", BASIC_TYPE_FUNC);
    putch.type.add_ret_or_arg_type(Type(BASIC_TYPE_NONE));
    putch.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    putch.addr = lib_start + 4;
    SymbolTableEntry putarray("putarray", BASIC_TYPE_FUNC);
    putarray.type.add_ret_or_arg_type(Type(BASIC_TYPE_NONE));
    putarray.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
    putarray.type.add_ret_or_arg_type(Type(BASIC_TYPE_INT, {0}));
    putarray.addr = lib_start + 5;
    SymbolTableEntry starttime("starttime", BASIC_TYPE_FUNC);
    starttime.type.add_ret_or_arg_type(Type(BASIC_TYPE_NONE));
    starttime.addr = lib_start + 6;
    SymbolTableEntry stoptime("stoptime", BASIC_TYPE_FUNC);
    stoptime.type.add_ret_or_arg_type(Type(BASIC_TYPE_NONE));
    stoptime.addr = lib_start + 7;
    symbol_table->add_entry(getint);
    symbol_table->add_entry(getch);
    symbol_table->add_entry(getarray);
    symbol_table->add_entry(putint);
    symbol_table->add_entry(putch);
    symbol_table->add_entry(putarray);
    symbol_table->add_entry(starttime);
    symbol_table->add_entry(stoptime);

    /* Lexing and parsing. */
    Parser parser(code);
    if (!parser.parse())
    {
        fprintf(stderr, "Parsing failed!\n");
        delete symbol_table;
        exit(2);
    }
    /* IR generation. */
    IntermediateCodeGenerator gen;
    gen.generate_code();
    if (gen.error)
    {
        fprintf(stderr, "Intermediate code generation failed!\n");
        delete symbol_table;
        exit(3);
    }

    auto code_out = gen.simplify_code();

    gen.print_code();

    /* Optimization and register allocation. */
    register_alloc_optim(code_out);

    /* Code generation. */
    CodeGenerator codegen(code_out);
    std::ofstream out_f(argv[4]);
    codegen.generate_code(out_f);
    out_f.close();
    for (auto& c: code_out)
    {
        delete c;
    }
    delete symbol_table;
    return 0;
}