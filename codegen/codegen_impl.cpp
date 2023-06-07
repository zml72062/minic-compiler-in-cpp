#include "codegen.h"

CodeGenerator::CodeGenerator(const std::vector<IntermediateCode*>& _code)
{
    procedures = make_procedures(_code);
}

CodeGenerator::~CodeGenerator()
{
    for (auto& p: procedures)
    {
        delete p;
    }
}

void CodeGenerator::generate_code(std::ostream& file)
{
    std::size_t func_idx = 0;
    for (auto& entry: symbol_table->get_entries())
    {
        if (entry->type.basic_type == BASIC_TYPE_INT)
        {
            file << "  .data\n"
                 << "  .globl " + entry->name + "\n"
                 << entry->name + ":\n";
            bool all_zero = true;
            for (auto& i: entry->init_val)
            {
                if (i != 0)
                {
                    all_zero = false;
                    break;
                }
            }
            if (all_zero)
            {
                file << "  .zero " + std::to_string(INT_SIZE * entry->init_val.size()) << "\n";
            }
            else
            {
                for (auto& i: entry->init_val)
                {
                    file << "  .word " + std::to_string(i) << "\n";
                }
            }
            file << "\n";
        }
        else if (entry->type.basic_type == BASIC_TYPE_FUNC)
        {
            auto func = procedures[func_idx++];
            file << "  .text\n"
                 << "  .globl " + entry->name + "\n"
                 << entry->name + ":\n";
            /* Generate function prologue. */
            auto stack_mov = func->stack_move_value(),
                 max_alloc = func->max_allocated_memory(),
                 max_spill = func->max_spilled_memory(),
                 max_args = func->max_call_args();
            std::size_t exceeding_args = 0;
            bool has_call = func->has_call();
            if (max_args > 8)
            {
                exceeding_args += (max_args - 8) * INT_SIZE;
            }
            if (has_call)
            {
                file << "  sw   s3, -8(sp)\n"
                     << "  sw   s2, -12(sp)\n"
                     << "  sw   s1, -16(sp)\n"
                     << "  sw   s0, -20(sp)\n"
                     << "  addi s0, sp, -20\n";
            }
            else
            {
                file << "  sw   s3, -4(sp)\n"
                     << "  sw   s2, -8(sp)\n"
                     << "  sw   s1, -12(sp)\n"
                     << "  sw   s0, -16(sp)\n"
                     << "  addi s0, sp, -16\n";
            }
            if (stack_mov <= 2048)
            {
                file << "  addi sp, sp, " + std::to_string(-(int)(stack_mov)) + "\n";
            }
            else
            {
                file << "  li   s3, " + std::to_string((int)stack_mov) + "\n"
                     << "  sub  sp, sp, s3\n";
            }
            std::size_t already_alloc = 0;
            for (auto& line: func->code)
            {
                file << to_asm(line, stack_mov, max_spill, exceeding_args, has_call, already_alloc) << "\n";
            }
            file << "\n";            
        }
    }
}

static std::string global_addr_to_str(std::size_t addr)
{
    for (auto& entry: symbol_table->get_entries())
    {
        if (entry->addr == addr)
        {
            return entry->name;
        }
    }
    return std::to_string(addr);
}

static std::string label_to_str(std::size_t label)
{
    if (label > (1 << 30))
        return global_addr_to_str(label) + "_start";
    return "L" + std::to_string(label);
}

static std::string reg_to_str(std::size_t reg)
{
    if (reg >= 1 && reg <= 7) /* t0 to t6 */
        return "t" + std::to_string(reg - 1);
    else if (reg >= 8 && reg <= 9) /* s1 to s2 */
        return "s" + std::to_string(reg - 7);
    return "%" + std::to_string(reg);
}

static std::string arg_to_str(std::size_t arg)
{
    if (arg >= 0 && arg <= 7)
        return "a" + std::to_string(arg);
    return "a";
}

std::string to_asm(IntermediateCode* code, 
                   std::size_t all_subtracted,
                   std::size_t all_spilled,
                   std::size_t all_exceeding_args,
                   int has_call,
                   std::size_t& already_allocated)
{
    auto labels = code->labels;
    auto dest = code->dest, instr = code->instr, 
         loperand = code->loperand, roperand = code->roperand;

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
            if (roperand)
                return prefix + "  la   " + reg_to_str(dest) + ", " + global_addr_to_str(loperand);
            else
                return prefix + "  li   " + reg_to_str(dest) + ", " + std::to_string((int)loperand);
        case INSTR_RRMOV:
            return prefix + "  mv   " + reg_to_str(dest) + ", " + reg_to_str(loperand);
        case INSTR_RMMOV:
            return prefix + "  sw   " + reg_to_str(loperand) + ", 0(" + reg_to_str(dest) + ")";
        case INSTR_MRMOV:
            return prefix + "  lw   " + reg_to_str(dest) + ", 0(" + reg_to_str(loperand) + ")";
        case INSTR_SAVE:
        {
            auto up_from_sp = all_exceeding_args + loperand;
            if (up_from_sp < 2048)
            {
                return prefix + "  sw   s1, " + std::to_string((int)up_from_sp) + "(sp)";
            }
            else
            {
                return prefix + "  li   s3, " + std::to_string((int)up_from_sp) + "\n"
                     + "  add  sp, sp, s3\n"
                     + "  sw   s1, 0(sp)\n"
                     + "  sub  sp, sp, s3";
            }
        }
        case INSTR_LOADD:
        {
            auto up_from_sp = all_exceeding_args + roperand;
            if (up_from_sp < 2048)
            {
                return prefix + "  lw   s1, " + std::to_string((int)up_from_sp) + "(sp)";
            }
            else
            {
                return prefix + "  li   s3, " + std::to_string((int)up_from_sp) + "\n"
                     + "  add  sp, sp, s3\n"
                     + "  lw   s1, 0(sp)\n"
                     + "  sub  sp, sp, s3";
            }
        }
        case INSTR_LOADO:
        {
            auto up_from_sp = all_exceeding_args + roperand;
            if (up_from_sp < 2048)
            {
                return prefix + "  lw   s2, " + std::to_string((int)up_from_sp) + "(sp)";
            }
            else
            {
                return prefix + "  li   s3, " + std::to_string((int)up_from_sp) + "\n"
                     + "  add  sp, sp, s3\n"
                     + "  lw   s2, 0(sp)\n"
                     + "  sub  sp, sp, s3";
            }
        }
        case INSTR_NEG:
            return prefix + "  sub  " + reg_to_str(dest) + ", x0, " + reg_to_str(loperand);
        case INSTR_NOT:
            return prefix + "  seqz " + reg_to_str(dest) + ", " + reg_to_str(loperand);
        case INSTR_BOOL:
            return prefix + "  snez " + reg_to_str(dest) + ", " + reg_to_str(loperand);
        case INSTR_ADD:
            return prefix + "  add  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_SUB:
            return prefix + "  sub  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_MUL:
            return prefix + "  mul  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_DIV:
            return prefix + "  div  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_MOD:
            return prefix + "  rem  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_GT:
            return prefix + "  sgt  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_GEQ:
            return prefix + "  slt  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand) + "\n"
                 + "  seqz " + reg_to_str(dest) + ", " + reg_to_str(dest);
        case INSTR_LT:
            return prefix + "  slt  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand);
        case INSTR_LEQ:
            return prefix + "  sgt  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand) + "\n"
                 + "  seqz " + reg_to_str(dest) + ", " + reg_to_str(dest);
        case INSTR_EQ:
            return prefix + "  xor  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand) + "\n"
                 + "  seqz " + reg_to_str(dest) + ", " + reg_to_str(dest);
        case INSTR_NEQ:
            return prefix + "  xor  " + reg_to_str(dest) + ", " + reg_to_str(loperand) + ", " + reg_to_str(roperand) + "\n"
                 + "  snez " + reg_to_str(dest) + ", " + reg_to_str(dest);
        case INSTR_ALLOC:
        {
            auto up_from_sp = all_exceeding_args + all_spilled + already_allocated;
            already_allocated += loperand;
            if (up_from_sp < 2048)
            {
                return prefix + "  addi " + reg_to_str(dest) + ", sp, " + std::to_string((int)up_from_sp);
            }
            else
            {
                return prefix + "  li   s3, " + std::to_string((int)up_from_sp) + "\n"
                     + "  add  " + reg_to_str(dest) + ", sp, s3";
            }
        }
        case INSTR_JMP:
            return prefix + "  j    " + label_to_str(roperand);
        case INSTR_JE:
            return prefix + "  beqz " + reg_to_str(loperand) + ", " + label_to_str(roperand);
        case INSTR_JNE:
            return prefix + "  bnez " + reg_to_str(loperand) + ", " + label_to_str(roperand);
        case INSTR_ARG:
        {
            if (loperand >= 0 && loperand <= 7)
                return prefix + "  mv   " + arg_to_str(loperand) + ", " + reg_to_str(roperand);
            else
            {
                auto up_from_sp = (loperand - 8) * INT_SIZE;
                return prefix + "  sw   " + reg_to_str(roperand) + ", " + std::to_string(up_from_sp) + "(sp)";
            }
        }
        case INSTR_LARG:
        {
            if (roperand >= 0 && roperand <= 7)
                return prefix + "  mv   " + reg_to_str(loperand) + ", " + arg_to_str(roperand);
            else
            {
                auto up_from_s0 = (roperand - 4 + has_call) * INT_SIZE;
                return prefix + "  lw   " + reg_to_str(loperand) + ", " + std::to_string(up_from_s0) + "(s0)";
            }
        }
        case INSTR_CALL:
            /* Save caller-saved registers. */
            return prefix + "  sw   ra, 16(s0)\n"
                          + "  sw   t0, -4(s0)\n"
                          + "  sw   t1, -8(s0)\n"
                          + "  sw   t2, -12(s0)\n"
                          + "  sw   t3, -16(s0)\n"
                          + "  sw   t4, -20(s0)\n"
                          + "  sw   t5, -24(s0)\n"
                          + "  sw   t6, -28(s0)\n"
                          + "  call " + global_addr_to_str(loperand) + "\n"
                          + "  lw   t0, -4(s0)\n"
                          + "  lw   t1, -8(s0)\n"
                          + "  lw   t2, -12(s0)\n"
                          + "  lw   t3, -16(s0)\n"
                          + "  lw   t4, -20(s0)\n"
                          + "  lw   t5, -24(s0)\n"
                          + "  lw   t6, -28(s0)\n"
                          + "  lw   ra, 16(s0)\n"
                          + "  mv   " + reg_to_str(dest) + ", a0";
        case INSTR_RET:
        {
            std::string epilogue(prefix);
            if (loperand > 0)
                epilogue += "  mv   a0,  " + reg_to_str(loperand) + "\n";
            if (all_subtracted < 2048)
            {
                epilogue += "  addi sp, sp, " + std::to_string((int)all_subtracted) + "\n";
            }
            else
            {
                epilogue += "  li   s3, " + std::to_string((int)all_subtracted) + "\n"
                          + "  add  sp, sp, s3\n";
            }
            /* Recover callee-saved registers. */
            epilogue += std::string("  lw   s3, 12(s0)\n")
                      + "  lw   s2, 8(s0)\n"
                      + "  lw   s1, 4(s0)\n"
                      + "  lw   s0, 0(s0)\n"
                      + "  ret  ";
            return epilogue;
        }
        default:
            return prefix;
    }
}