#include "basicblock.h"
#include <queue>

AllocationTable::AllocationTable(std::pair<std::size_t, std::size_t> _register_range): 
allocation_table(_register_range.second - _register_range.first)
{
    for (auto i = _register_range.first; i < _register_range.second; i++)
    {
        allocation_table[i - _register_range.first] = i + (1 << 29);
    }
    register_lower_range = _register_range.first;
    free_registers = std::set<std::size_t>({1, 2, 3, 4, 5, 6, 7});
}

const std::size_t& AllocationTable::operator[](std::size_t reg) const
{
    return this->allocation_table[reg - register_lower_range];
}

std::size_t& AllocationTable::operator[](std::size_t reg)
{
    return this->allocation_table[reg - register_lower_range];
}

RegisterModifier::RegisterModifier(const AllocationTable& _alloc_table): alloc_table(_alloc_table)
{

}

static void modify_code_use(IntermediateCode* code, const AllocationTable& alloc_table)
{
    switch (code->instr)
    {
        case INSTR_IRMOV:
            return;
        case INSTR_RRMOV:
            code->loperand = alloc_table[code->loperand];
            return;
        case INSTR_RMMOV:
            code->loperand = alloc_table[code->loperand];
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_MRMOV:
            code->loperand = alloc_table[code->loperand];
            return;
        case INSTR_ALLOC:
            return;
        case INSTR_NEG:
        case INSTR_NOT:
        case INSTR_BOOL:
            code->loperand = alloc_table[code->loperand];
            return;
        case INSTR_ADD:
        case INSTR_SUB:
        case INSTR_MUL:
        case INSTR_DIV:
        case INSTR_MOD:
        case INSTR_GT:
        case INSTR_GEQ:
        case INSTR_LT:
        case INSTR_LEQ:
        case INSTR_EQ:
        case INSTR_NEQ:
            code->loperand = alloc_table[code->loperand];
            code->roperand = alloc_table[code->roperand];
            return;
        case INSTR_JMP:
            return;
        case INSTR_JE:
        case INSTR_JNE:
            code->loperand = alloc_table[code->loperand];
            return;
        case INSTR_ARG:
            code->roperand = alloc_table[code->roperand];
            return;
        case INSTR_LARG:
            return;
        case INSTR_CALL:
            return;
        case INSTR_RET:
            if (code->loperand == 0)
                return;
            code->loperand = alloc_table[code->loperand];
            return;
        case INSTR_GLOB:
        default:
            return;
    }
}

static void modify_code_def(IntermediateCode* code, const AllocationTable& alloc_table)
{
    switch (code->instr)
    {
        case INSTR_IRMOV:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_RRMOV:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_RMMOV:
            return;
        case INSTR_MRMOV:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_ALLOC:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_NEG:
        case INSTR_NOT:
        case INSTR_BOOL:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_ADD:
        case INSTR_SUB:
        case INSTR_MUL:
        case INSTR_DIV:
        case INSTR_MOD:
        case INSTR_GT:
        case INSTR_GEQ:
        case INSTR_LT:
        case INSTR_LEQ:
        case INSTR_EQ:
        case INSTR_NEQ:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_JMP:
        case INSTR_JE:
        case INSTR_JNE:
        case INSTR_ARG:
            return;
        case INSTR_LARG:
            code->loperand = alloc_table[code->loperand];
            return;
        case INSTR_CALL:
            code->dest = alloc_table[code->dest];
            return;
        case INSTR_RET:
        case INSTR_GLOB:
        default:
            return;
    }
}

void RegisterModifier::modify(IntermediateCode* _code, 
                              const std::set<std::size_t>& _before,
                              const std::set<std::size_t>& _after)
{
    modify_code_use(_code, this->alloc_table);

    /* 'unused' is the set of formal registers that are not alive after the 
       instruction; 
       'needed' is the set of formal registers requiring allocation for the
       instruction. */
    std::set<std::size_t> unused, needed;
    for (auto& p: _before)
    {
        if (_after.find(p) == _after.end()) /* p not in '_after' */
            unused.insert(p);
    }
    for (auto& p: _after)
    {
        if (_before.find(p) == _before.end())
            needed.insert(p);
    }
    for (auto& l: unused)
    {
        auto reg = this->alloc_table[l];
        if (reg <= (1 << 29))
        {
            this->alloc_table.free_registers.insert(reg);
            this->alloc_table[l] = (1 << 29) + l;
        }
    }

    for (auto& v: needed)
    {
        if (alloc_table.free_registers.empty()) /* Spill to memory. */
            this->alloc_table[v] = (1 << 29) + v;
        else
        {
            auto min_reg = *alloc_table.free_registers.begin();
            alloc_table.free_registers.erase(min_reg);
            alloc_table[v] = min_reg;
        }
    }

    modify_code_def(_code, this->alloc_table);
}

RegisterAllocator::RegisterAllocator(const AllocationTable& _alloc_table): modifier(_alloc_table)
{

}

void RegisterAllocator::allocate(const std::vector<IntermediateCode*>& _code,
                                 const std::vector<std::set<std::size_t>>& _global_liveness)
{
    auto num_lines = _code.size();
    for (std::size_t i = 0; i < num_lines; i++)
    {
        modifier.modify(_code[i], _global_liveness[i], _global_liveness[i + 1]);
    }
}