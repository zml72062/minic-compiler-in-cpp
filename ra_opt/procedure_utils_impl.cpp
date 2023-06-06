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