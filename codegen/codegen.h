#ifndef CODEGEN_H
#define CODEGEN_H

#include "../parse/symtab.h"
#include "../intermediate/intermediate.h"
#include "../ra_opt/basicblock.h"
#include <iostream>

extern SymbolTable* symbol_table;

std::string to_asm(IntermediateCode* code, 
                   std::size_t all_subtracted,
                   std::size_t all_spilled,
                   std::size_t all_exceeding_args,
                   int has_call,
                   std::size_t& already_allocated);

struct CodeGenerator
{
    std::vector<Procedure*> procedures;
    CodeGenerator(const std::vector<IntermediateCode*>& _code);
    ~CodeGenerator();

    CodeGenerator(const CodeGenerator& _g) = delete;
    CodeGenerator(CodeGenerator&& _g) = delete;
    CodeGenerator& operator=(const CodeGenerator& _g) = delete;
    CodeGenerator& operator=(CodeGenerator&& _g) = delete;

    void generate_code(std::ostream& file);
};

#endif