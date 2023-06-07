#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include "../intermediate/intermediate.h"
#include "../parse/symtab.h"
#include <set>
#include <map>

extern SymbolTable* symbol_table;

struct Procedure
{
    std::vector<IntermediateCode*> code;
    Procedure(const std::vector<IntermediateCode*>& _code);
    void print_code();
    std::pair<std::size_t, std::size_t> register_range();
    bool has_call();
    std::size_t max_call_args();
    std::size_t max_spilled_memory();
    std::size_t max_allocated_memory();
    std::size_t stack_move_value();
};

std::vector<Procedure*> make_procedures(const std::vector<IntermediateCode*>& code);

struct BasicBlock
{
    BasicBlock(const std::vector<IntermediateCode*>& _code, std::pair<std::size_t, std::size_t> _line_range);
    std::pair<std::size_t, std::size_t> line_range;
    std::vector<std::size_t> predecessors;
    std::vector<std::size_t> successors;
    std::vector<IntermediateCode*> code;
    void print_code();
};

std::vector<BasicBlock*> make_basic_blocks(const std::vector<IntermediateCode*>& code);

/**** Helper class for Liveness Analysis ****/

struct LivenessUpdater
{
    std::vector<BasicBlock*> basic_blocks;
    /* Register liveness between every two instructions of each basic block. */
    std::vector<std::vector<std::set<std::size_t>>> liveness;
    LivenessUpdater(const std::vector<BasicBlock*>& _basic_blocks);
    std::vector<std::vector<std::set<std::size_t>>> iterate_liveness();
    void calculate_liveness();
    std::vector<std::set<std::size_t>> to_global_liveness(std::size_t code_length);
};

/**** Helper class for Register Allocation ****/

struct AllocationTable
{
    /* 1-7 means t0-t6, (1 << 30) means memory */
    std::vector<std::size_t> allocation_table;
    std::size_t register_lower_range;
    std::set<std::size_t> free_registers;
    AllocationTable(std::pair<std::size_t, std::size_t> _register_range);
    const std::size_t& operator[](std::size_t reg) const;
    std::size_t& operator[](std::size_t reg);
};

struct RegisterModifier
{
    AllocationTable alloc_table;
    RegisterModifier(const AllocationTable& _alloc_table);
    void modify(IntermediateCode* _code, const std::set<std::size_t>& _before,
                const std::set<std::size_t>& _after);
};

struct RegisterAllocator
{
    RegisterModifier modifier;
    RegisterAllocator(const AllocationTable& _alloc_table);
    void allocate(const std::vector<IntermediateCode*>& _code, 
                  const std::vector<std::set<std::size_t>>& _global_liveness);
};

struct MemorySpiller
{
    std::size_t addr;
    std::map<std::size_t, std::size_t> memory_map;
    MemorySpiller();
    std::size_t next_addr();
    void spill(std::vector<IntermediateCode*>& code);
};

void remove_useless_mov(std::vector<IntermediateCode*>& code);
void register_alloc_optim(std::vector<IntermediateCode*>& code);

#endif