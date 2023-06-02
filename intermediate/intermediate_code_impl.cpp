#include "intermediate.h"

IntermediateCode::IntermediateCode(std::size_t _instr, std::size_t _dest, std::size_t _loperand, std::size_t _roperand)
{
    loperand = _loperand;
    roperand = _roperand;
    dest = _dest;
    instr = _instr;
    label = 0;
}

IntermediateCode::IntermediateCode(std::size_t _instr, std::size_t _dest, std::size_t _loperand, std::size_t _roperand, std::size_t& _label)
{
    loperand = _loperand;
    roperand = _roperand;
    dest = _dest;
    instr = _instr;
    label = _label;
    _label = 0;
}

static std::string addr_to_str(std::size_t addr)
{
    return "%" + std::to_string(addr);
}

static std::string arg_to_str(std::size_t addr)
{
    return "%arg" + std::to_string(addr);
}

std::string IntermediateCode::to_str()
{
    std::string prefix;
    if (label > 0)
    {
        prefix += ("L" + std::to_string(label) + ":\n");
    }
    switch (instr)
    {
        case INSTR_IRMOV:
            return prefix + "  irmov  " + addr_to_str(dest) + ", " + std::to_string(loperand);
        case INSTR_RRMOV:
            return prefix + "  rrmov  " + addr_to_str(dest) + ", " + addr_to_str(loperand);
        case INSTR_RMMOV:
            return prefix + "  rmmov  (" + addr_to_str(dest) + "), " + addr_to_str(loperand);
        case INSTR_MRMOV:
            return prefix + "  mrmov  " + addr_to_str(dest) + ", (" + addr_to_str(loperand) + ")";
        case INSTR_ALLOC:
            return prefix + "  alloc  (" + addr_to_str(dest) + "), " + std::to_string(loperand);
        case INSTR_NEG:
            return prefix + "  neg    " + addr_to_str(dest) + ", " + addr_to_str(loperand);
        case INSTR_NOT:
            return prefix + "  not    " + addr_to_str(dest) + ", " + addr_to_str(loperand);
        case INSTR_ADD:
            return prefix + "  add    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_SUB:
            return prefix + "  sub    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_MUL:
            return prefix + "  mul    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_DIV:
            return prefix + "  div    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_MOD:
            return prefix + "  mod    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_GT:
            return prefix + "  gt     " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_GEQ:
            return prefix + "  geq    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_LT:
            return prefix + "  lt     " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_LEQ:
            return prefix + "  leq    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_EQ:
            return prefix + "  eq     " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_NEQ:
            return prefix + "  neq    " + addr_to_str(dest) + ", " + addr_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_JMP:
            return prefix + "  jmp    " + "L" + std::to_string(roperand);
        case INSTR_JE:
            return prefix + "  je     " + addr_to_str(loperand) + ", " + "L" + std::to_string(roperand);
        case INSTR_JNE:
            return prefix + "  jne    " + addr_to_str(loperand) + ", " + "L" + std::to_string(roperand);
        case INSTR_ARG:
            return prefix + "  arg    " + arg_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_CALL:
            return prefix + "  call   " + addr_to_str(dest) + ", " + "L" + std::to_string(loperand) + ", " + std::to_string(roperand);
        case INSTR_RET:
            return prefix + "  ret    " + addr_to_str(loperand);
        default:
            return prefix;
    }
}