#include "basicblock.h"

std::pair<std::size_t, std::size_t> Procedure::register_range()
{
    std::size_t min_ = (1 << 29), max_ = 0;
    for (auto& code_line: code)
    {
        switch (code_line->instr)
        {
            case INSTR_IRMOV:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                break;
            case INSTR_RRMOV:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_RMMOV:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_MRMOV:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_ALLOC:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                break;
            case INSTR_NEG:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_NOT:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_ADD:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_SUB:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_MUL:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_DIV:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_MOD:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_GT:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_GEQ:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_LT:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_LEQ:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_EQ:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_NEQ:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_JMP:
                break;
            case INSTR_JE:
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_JNE:
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_ARG:
                if (code_line->roperand < min_)
                    min_ = code_line->roperand;
                if (code_line->roperand > max_)
                    max_ = code_line->roperand;
                break;
            case INSTR_LARG:
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_CALL:
                if (code_line->dest < min_)
                    min_ = code_line->dest;
                if (code_line->dest > max_)
                    max_ = code_line->dest;
                break;
            case INSTR_RET:
                if (code_line->loperand == 0)
                    break;
                if (code_line->loperand < min_)
                    min_ = code_line->loperand;
                if (code_line->loperand > max_)
                    max_ = code_line->loperand;
                break;
            case INSTR_GLOB:
            default:
                break;
        }
    }
    return {min_, max_ + 1};
}

bool Procedure::has_call()
{
    for (auto& code_line: code)
    {
        if (code_line->instr == INSTR_CALL)
            return true;
    }
    return false;
}

std::size_t Procedure::max_call_args()
{
    std::size_t max_args = 0;
    for (auto& code_line: code)
    {
        if (code_line->instr == INSTR_CALL && code_line->roperand > max_args)
            max_args = code_line->roperand;
    }
    return max_args;
}

std::size_t Procedure::max_spilled_memory()
{
    std::size_t memory_min = (1 << 30), memory_max = 0;
    for (auto& code_line: code)
    {
        if (code_line->instr == INSTR_SAVE)
        {
            auto rel_addr = code_line->loperand;
            if (rel_addr < memory_min)
                memory_min = rel_addr;
            if (rel_addr > memory_max)
                memory_max = rel_addr;
        }
        else if (code_line->instr == INSTR_LOADD ||
                 code_line->instr == INSTR_LOADO)
        {
            auto rel_addr = code_line->roperand;
            if (rel_addr < memory_min)
                memory_min = rel_addr;
            if (rel_addr > memory_max)
                memory_max = rel_addr;
        }
    }
    if (memory_max >= memory_min)
        return memory_max - memory_min + INT_SIZE;
    else
        return 0;
}

std::size_t Procedure::max_allocated_memory()
{
    std::size_t memory = 0;
    for (auto& code_line: code)
    {
        if (code_line->instr == INSTR_ALLOC)
        {
            memory += code_line->loperand;
        }
    }
    return memory;
}

std::size_t Procedure::stack_move_value()
{
    /** The stack frame is like:
     * 
     *  --------------------------
     *       (in ra register)
     *        return address
     *   (after current function
     *         call finishes)
     *
     *    0 bytes if "is_leaf()",
     *    otherwise PTR_SIZE.
     *  --------------------------
     *      saved s3 register
     *  --------------------------
     *      saved s2 register
     *      (as OPERAND_TEMP)
     *  --------------------------
     *      saved s1 register
     *       (as DEST_TEMP)
     *  --------------------------
     *      saved s0 register
     *      (as frame pointer)
     *  --------------------------   <---- s0 register here
     *     place to save t0-t6
     *    
     *    0 bytes if "is_leaf()",
     *    otherwise INT_SIZE * 7.
     *  --------------------------
     *     potential free space
     *       due to alignment
     *  --------------------------
     *       allocated_memory
     *  --------------------------
     *        spilled_memory
     *  --------------------------
     *       place to save >8 
     *           func args
     *  --------------------------   <---- sp register here
     */
    std::size_t move_value = max_spilled_memory() + 
                             max_allocated_memory() +
                             4 * INT_SIZE;
    auto max_args = max_call_args();
    if (max_args > 8)
    {
        move_value += (max_args - 8) * INT_SIZE;
    }
    if (has_call())
    {
        move_value += (PTR_SIZE + 7 * INT_SIZE);
    }
    if (move_value % 16 == 0)
        return move_value;
    else
        return (1 + move_value / 16) * 16;
}