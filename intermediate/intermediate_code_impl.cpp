#include "intermediate.h"

IntermediateCode::IntermediateCode(std::size_t _instr, std::size_t _dest, std::size_t _loperand, std::size_t _roperand)
{
    loperand = _loperand;
    roperand = _roperand;
    dest = _dest;
    instr = _instr;
}

IntermediateCode::IntermediateCode(std::size_t _instr, std::size_t _dest, std::size_t _loperand, std::size_t _roperand, std::vector<std::size_t>& _label)
{
    loperand = _loperand;
    roperand = _roperand;
    dest = _dest;
    instr = _instr;
    labels = _label;
    _label = std::vector<std::size_t>();
}

static bool is_global(std::size_t addr_or_label)
{
    return ((int)addr_or_label) > (1 << 30);
}

static std::string addr_to_str(std::size_t addr)
{
    if (is_global(addr))
    {
        for (auto& entry: symbol_table->get_entries())
        {
            if (entry->addr == addr)
            {
                return entry->name;
            }
        }
    }
    return "%" + std::to_string(addr);
}

static std::string arg_to_str(std::size_t addr)
{
    return "%arg" + std::to_string(addr);
}

static std::string label_to_str(std::size_t label)
{
    if (is_global(label))
    {
        for (auto& entry: symbol_table->get_entries())
        {
            if (entry->addr == label)
            {
                return entry->name;
            }
        }
    }
    return "L" + std::to_string(label);
}

std::string IntermediateCode::to_str()
{
    std::string prefix;
    for (auto& label: labels)
    {
        if (label > 0)
        {
            prefix += (label_to_str(label) + ":\n");
        }
    }
    switch (instr)
    {
        case INSTR_IRMOV:
            if (is_global(loperand))
                return prefix + "  irmov  " + addr_to_str(dest) + ", " + addr_to_str(loperand);
            else
                return prefix + "  irmov  " + addr_to_str(dest) + ", " + std::to_string((int)loperand);
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
            return prefix + "  jmp    " + label_to_str(roperand);
        case INSTR_JE:
            return prefix + "  je     " + addr_to_str(loperand) + ", " + label_to_str(roperand);
        case INSTR_JNE:
            return prefix + "  jne    " + addr_to_str(loperand) + ", " + label_to_str(roperand);
        case INSTR_ARG:
            return prefix + "  arg    " + arg_to_str(loperand) + ", " + addr_to_str(roperand);
        case INSTR_LARG:
            return prefix + "  larg   " + addr_to_str(loperand) + ", " + arg_to_str(roperand);
        case INSTR_CALL:
            return prefix + "  call   " + addr_to_str(dest) + ", " + label_to_str(loperand) + ", " + std::to_string(roperand);
        case INSTR_RET:
            return prefix + "  ret    " + addr_to_str(loperand);
        case INSTR_GLOB:
            return prefix + "  glob   " + addr_to_str(loperand);
        default:
            return prefix;
    }
}