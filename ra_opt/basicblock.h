#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include "../intermediate/intermediate.h"
#include "../parse/symtab.h"

extern SymbolTable* symbol_table;

struct Procedure
{
    std::vector<IntermediateCode*> code;
    Procedure(const std::vector<IntermediateCode*>& _code);
    void print_code();
};

std::vector<Procedure*> make_procedures(const std::vector<IntermediateCode*>& code);

struct BasicBlock
{
    BasicBlock(const std::vector<IntermediateCode*>& _code);
    std::vector<BasicBlock*> predecessors;
    std::vector<BasicBlock*> successors;
    std::vector<IntermediateCode*> code;
    void print_code();
};

std::vector<BasicBlock*> make_basic_blocks(const std::vector<IntermediateCode*>& code);

#endif