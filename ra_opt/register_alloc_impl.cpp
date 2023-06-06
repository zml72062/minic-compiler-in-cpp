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
            this->alloc_table.free_registers.insert(reg);
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

RegisterAllocator::RegisterAllocator(const AllocationTable& _alloc_table): alloc_table(_alloc_table)
{

}

void RegisterAllocator::allocate(LivenessUpdater& _updater)
{
    auto blocks = _updater.basic_blocks;
    auto liveness = _updater.liveness;
    /* Breadth first search. */
    std::queue<std::size_t> q;
    std::queue<AllocationTable*> tables;
    q.push(0);
    tables.push(new AllocationTable(alloc_table));
    std::set<std::size_t> visited;
    visited.insert(0);
    while (!q.empty())
    {
        auto cur_block_idx = q.front();
        auto cur_table = tables.front();
        q.pop();
        tables.pop();

        RegisterModifier r(*cur_table);
        auto num_lines = blocks[cur_block_idx]->code.size();
        for (std::size_t i = 0; i < num_lines; i++)
        {
            r.modify(blocks[cur_block_idx]->code[i],
                     liveness[cur_block_idx][num_lines - i],
                     liveness[cur_block_idx][num_lines - i - 1]);
        }

        for (auto& s: blocks[cur_block_idx]->successors)
        {
            if (visited.find(s) == visited.end()) /* s not in 'visited' */
            {
                q.push(s);
                visited.insert(s);
                tables.push(new AllocationTable(r.alloc_table));
            }
        }
        delete cur_table;
    }
}