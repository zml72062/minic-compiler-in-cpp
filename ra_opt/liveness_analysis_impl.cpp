#include "basicblock.h"

LivenessUpdater::LivenessUpdater(const std::vector<BasicBlock*>& _basic_blocks)
{
    basic_blocks = _basic_blocks;
    auto length = basic_blocks.size();
    liveness = std::vector<std::vector<std::set<std::size_t>>>(length);
    for (std::size_t i = 0; i < length; i++)
    {
        auto liveinfo_len = basic_blocks[i]->code.size() + 1;
        liveness[i] = std::vector<std::set<std::size_t>>(liveinfo_len, std::set<std::size_t>());
    }
}

static std::set<std::size_t> use(IntermediateCode* code)
{
    switch (code->instr)
    {
        case INSTR_IRMOV:
            return std::set<std::size_t>();
        case INSTR_RRMOV:
            return std::set<std::size_t>({code->loperand});
        case INSTR_RMMOV:
            return std::set<std::size_t>({code->dest, code->loperand});
        case INSTR_MRMOV:
            return std::set<std::size_t>({code->loperand});
        case INSTR_ALLOC:
            return std::set<std::size_t>();
        case INSTR_NEG:
            return std::set<std::size_t>({code->loperand});
        case INSTR_NOT:
            return std::set<std::size_t>({code->loperand});
        case INSTR_ADD:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_SUB:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_MUL:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_DIV:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_MOD:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_GT:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_GEQ:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_LT:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_LEQ:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_EQ:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_NEQ:
            return std::set<std::size_t>({code->loperand, code->roperand});
        case INSTR_JMP:
            return std::set<std::size_t>();
        case INSTR_JE:
            return std::set<std::size_t>({code->loperand});
        case INSTR_JNE:
            return std::set<std::size_t>({code->loperand});
        case INSTR_ARG:
            return std::set<std::size_t>({code->roperand});
        case INSTR_LARG:
            return std::set<std::size_t>();
        case INSTR_CALL:
            return std::set<std::size_t>();
        case INSTR_RET:
            if (code->loperand == 0)
                return std::set<std::size_t>();
            return std::set<std::size_t>({code->loperand});
        case INSTR_GLOB:
            return std::set<std::size_t>();
        default:
            return std::set<std::size_t>();
    }
}

static std::set<std::size_t> def(IntermediateCode* code)
{
    switch (code->instr)
    {
        case INSTR_IRMOV:
            return std::set<std::size_t>({code->dest});
        case INSTR_RRMOV:
            return std::set<std::size_t>({code->dest});
        case INSTR_RMMOV:
            return std::set<std::size_t>();
        case INSTR_MRMOV:
            return std::set<std::size_t>({code->dest});
        case INSTR_ALLOC:
            return std::set<std::size_t>({code->dest});
        case INSTR_NEG:
            return std::set<std::size_t>({code->dest});
        case INSTR_NOT:
            return std::set<std::size_t>({code->dest});
        case INSTR_ADD:
            return std::set<std::size_t>({code->dest});
        case INSTR_SUB:
            return std::set<std::size_t>({code->dest});
        case INSTR_MUL:
            return std::set<std::size_t>({code->dest});
        case INSTR_DIV:
            return std::set<std::size_t>({code->dest});
        case INSTR_MOD:
            return std::set<std::size_t>({code->dest});
        case INSTR_GT:
            return std::set<std::size_t>({code->dest});
        case INSTR_GEQ:
            return std::set<std::size_t>({code->dest});
        case INSTR_LT:
            return std::set<std::size_t>({code->dest});
        case INSTR_LEQ:
            return std::set<std::size_t>({code->dest});
        case INSTR_EQ:
            return std::set<std::size_t>({code->dest});
        case INSTR_NEQ:
            return std::set<std::size_t>({code->dest});
        case INSTR_JMP:
            return std::set<std::size_t>();
        case INSTR_JE:
            return std::set<std::size_t>();
        case INSTR_JNE:
            return std::set<std::size_t>();
        case INSTR_ARG:
            return std::set<std::size_t>();
        case INSTR_LARG:
            return std::set<std::size_t>({code->loperand});
        case INSTR_CALL:
            return std::set<std::size_t>({code->dest});
        case INSTR_RET:
            return std::set<std::size_t>();
        case INSTR_GLOB:
            return std::set<std::size_t>();
        default:
            return std::set<std::size_t>();
    }
}

std::vector<std::vector<std::set<std::size_t>>> LivenessUpdater::iterate_liveness()
{
    auto length = basic_blocks.size();
    /* For basic block i, new_liveness[i] is the liveness information of
       that block, in reversed order (i.e. new_liveness[i][0] is the liveness
       at the exit point of the block). */
    std::vector<std::vector<std::set<std::size_t>>> new_liveness(liveness);
    /* Traverse blocks reversely. */
    for (std::size_t n = length; n > 0; n--)
    {
        /* For EXIT block, 'liveness' is always empty. */
        if (n == length)
        {
            new_liveness[n - 1] = std::vector<std::set<std::size_t>>();
            new_liveness[n - 1].push_back({});
            continue;
        }
        /* Now we update new_liveness[n - 1], i.e. the liveness information
           about basic block (n - 1). */
        auto to_union = std::set<std::size_t>();
        /* At the exit point of block "n - 1", the live variables is the union
           of all live variables at the entry points of all successors of block
           "n - 1". */
        for (auto& block_idx: basic_blocks[n - 1]->successors)
        {
            /* new_liveness[block_idx] is the liveness information of the successor
               "block_idx",
               basic_blocks[block_idx]->code.size() is the largest available index
               of the liveness information of "block_idx". 
               Therefore, the RHS is the liveness variables at the BEGINNING of block
               "block_idx". */
            auto component = new_liveness[block_idx][basic_blocks[block_idx]->code.size()];
            to_union.insert(component.begin(), component.end());
        }
        new_liveness[n - 1] = std::vector<std::set<std::size_t>>();
        new_liveness[n - 1].push_back(to_union);

        auto code_length = basic_blocks[n - 1]->code.size();
        for (auto k = code_length; k > 0; k--)
        {
            auto code_line = basic_blocks[n - 1]->code[k - 1];

            auto use_ = use(code_line);
            auto def_ = def(code_line);
            std::set<std::size_t> without_def;
            std::set<std::size_t> with_use;
            /* without_def = new_liveness[n - 1].back() - def_ */
            for (auto& p: new_liveness[n - 1].back())
            {
                if (def_.find(p) == def_.end()) 
                    without_def.insert(p);
            }
            /* with_use = without_def + use_ */
            for (auto& p: without_def)
            {
                with_use.insert(p);
            }
            for (auto& p: use_)
            {
                with_use.insert(p);
            }
            new_liveness[n - 1].push_back(with_use);
        }
    }

    return new_liveness;
}

void LivenessUpdater::calculate_liveness()
{
    std::vector<std::vector<std::set<std::size_t>>> new_liveness = liveness;
    do
    {
        liveness = new_liveness;
        new_liveness = iterate_liveness();
    } while (new_liveness != liveness);
}