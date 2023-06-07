#include "basicblock.h"

static bool is_memory(std::size_t addr_or_label)
{
    return ((int)addr_or_label) > (1 << 29);
}

MemorySpiller::MemorySpiller()
{
    addr = 0;
}

std::size_t MemorySpiller::next_addr()
{
    return addr++;
}

void MemorySpiller::spill(std::vector<IntermediateCode*>& code)
{
    auto length = code.size();
    for (std::size_t i = 0; i < length; i++)
    {
        auto code_line = code[i];
        auto dest = code_line->dest, loperand = code_line->loperand,
            roperand = code_line->roperand, instr = code_line->instr;
        std::vector<std::size_t> labels(code_line->labels);
        switch (instr)
        {
            case INSTR_ALLOC:
            case INSTR_IRMOV:
            case INSTR_CALL:
                if (is_memory(dest))
                {
                    delete code_line;
                    code.erase(code.begin() + i);
                    /* Move immediate to DEST_TEMP */
                    code.insert(code.begin() + i,
                    new IntermediateCode(instr, DEST_TEMP, loperand, roperand, labels));
                    /* Save DEST_TEMP to dest */
                    try
                    {
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                    } catch (const std::out_of_range& e)
                    {
                        
                        memory_map[dest] = 4 * next_addr();
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                    }
                    i++;
                    length++;
                }
                break;
            case INSTR_LARG:
                if (is_memory(loperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);
                    code.insert(code.begin() + i,
                    new IntermediateCode(instr, PLACEHOLDER, DEST_TEMP, roperand, labels));
                    /* Save DEST_TEMP to dest */
                    try
                    {
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(loperand), PLACEHOLDER, labels));
                    } catch (const std::out_of_range& e)
                    {
                        
                        memory_map[loperand] = 4 * next_addr();
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(loperand), PLACEHOLDER, labels));
                    }
                    i++;
                    length++;
                }
                break;
            case INSTR_RRMOV:
                if (is_memory(dest) || is_memory(loperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);

                    /* Move 'loperand' to DEST_TEMP */
                    if (is_memory(loperand))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                    }
                    else
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_RRMOV, DEST_TEMP, loperand, roperand, labels));
                    }
                    
                    /* Move DEST_TEMP to dest */
                    if (is_memory(dest))
                    {
                        try
                        {
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        } catch (const std::out_of_range& e)
                        {
                            
                            memory_map[dest] = 4 * next_addr();
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        }
                    }
                    else
                    {
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_RRMOV, dest, DEST_TEMP, roperand, labels));
                    }

                    i++;
                    length++;
                }
                break;
            case INSTR_MRMOV:
            case INSTR_NEG:
            case INSTR_NOT:
                if (is_memory(dest) || is_memory(loperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);

                    if (is_memory(loperand) && is_memory(dest))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(instr, DEST_TEMP, DEST_TEMP, roperand, labels));
                        try
                        {
                            code.insert(code.begin() + i + 2,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        } catch (const std::out_of_range& e)
                        {
                            
                            memory_map[dest] = 4 * next_addr();
                            code.insert(code.begin() + i + 2,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        }
                        i += 2;
                        length += 2;
                    }
                    else if (is_memory(loperand) && !is_memory(dest))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(instr, dest, DEST_TEMP, roperand, labels));
                        i++;
                        length++;
                    }
                    else if (is_memory(dest) && !is_memory(loperand))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(instr, DEST_TEMP, loperand, roperand, labels));
                        try
                        {
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        } catch (const std::out_of_range& e)
                        {
                            
                            memory_map[dest] = 4 * next_addr();
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        }
                        i++;
                        length++;
                    }
                }
                break;
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
                if (is_memory(dest) || is_memory(loperand) || is_memory(roperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);
                    if (is_memory(loperand) && is_memory(roperand))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADO, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(roperand), labels));
                        if (is_memory(dest))
                        {
                            code.insert(code.begin() + i + 2,
                            new IntermediateCode(instr, DEST_TEMP, OPERAND_TEMP, DEST_TEMP, labels));
                            try
                            {
                                code.insert(code.begin() + i + 3,
                                new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                            } catch (const std::out_of_range& e)
                            {
                                
                                memory_map[dest] = 4 * next_addr();
                                code.insert(code.begin() + i + 3,
                                new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                            }
                            i += 3;
                            length += 3;
                        }
                        else
                        {
                            code.insert(code.begin() + i + 2,
                            new IntermediateCode(instr, dest, OPERAND_TEMP, DEST_TEMP, labels));
                            i += 2;
                            length += 2;
                        }
                    }
                    else if (is_memory(loperand) && !is_memory(roperand))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADO, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                        if (is_memory(dest))
                        {
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(instr, DEST_TEMP, OPERAND_TEMP, roperand, labels));
                            try
                            {
                                code.insert(code.begin() + i + 2,
                                new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                            } catch (const std::out_of_range& e)
                            {
                                
                                memory_map[dest] = 4 * next_addr();
                                code.insert(code.begin() + i + 2,
                                new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                            }
                            i += 2;
                            length += 2;
                        }
                        else
                        {
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(instr, dest, OPERAND_TEMP, roperand, labels));
                            i++;
                            length++;
                        }
                    }
                    else if (!is_memory(loperand) && is_memory(roperand))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(roperand), labels));
                        if (is_memory(dest))
                        {
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(instr, DEST_TEMP, loperand, DEST_TEMP, labels));
                            try
                            {
                                code.insert(code.begin() + i + 2,
                                new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                            } catch (const std::out_of_range& e)
                            {
                                
                                memory_map[dest] = 4 * next_addr();
                                code.insert(code.begin() + i + 2,
                                new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                            }
                            i += 2;
                            length += 2;
                        }
                        else
                        {
                            printf("Here %lu\n", roperand - (1 << 29));
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(instr, dest, loperand, DEST_TEMP, labels));
                            printf("%s\n", (*(code.begin() + i + 1))->to_str().c_str());
                            i++;
                            length++;
                        }
                    }
                    else
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(instr, DEST_TEMP, loperand, roperand, labels));
                        try
                        {
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        } catch (const std::out_of_range& e)
                        {
                            
                            memory_map[dest] = 4 * next_addr();
                            code.insert(code.begin() + i + 1,
                            new IntermediateCode(INSTR_SAVE, PLACEHOLDER, memory_map.at(dest), PLACEHOLDER, labels));
                        }
                        i++;
                        length++;
                    }
                }
                break;

            case INSTR_RMMOV:
                if (is_memory(dest) || is_memory(loperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);

                    if (is_memory(loperand) && is_memory(dest))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADO, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(dest), labels));
                        code.insert(code.begin() + i + 2,
                        new IntermediateCode(INSTR_RMMOV, DEST_TEMP, OPERAND_TEMP, roperand, labels));
                        i += 2;
                        length += 2;
                    }
                    else if (is_memory(loperand) && !is_memory(dest))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADO, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_RMMOV, dest, OPERAND_TEMP, roperand, labels));
                        i++;
                        length++;
                    }
                    else if (is_memory(dest) && !is_memory(loperand))
                    {
                        code.insert(code.begin() + i,
                        new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(dest), labels));
                        code.insert(code.begin() + i + 1,
                        new IntermediateCode(INSTR_RMMOV, DEST_TEMP, loperand, roperand, labels));
                        i++;
                        length++;
                    }
                }
                break;
            case INSTR_JE:
            case INSTR_JNE:
            case INSTR_RET:
                if (is_memory(loperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);

                    code.insert(code.begin() + i,
                    new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(loperand), labels));
                    code.insert(code.begin() + i + 1,
                    new IntermediateCode(instr, dest, DEST_TEMP, roperand, labels));
                    i++;
                    length++;
                }
                break;
            case INSTR_ARG:
                if (is_memory(roperand))
                {
                    delete code_line;
                    code.erase(code.begin() + i);

                    code.insert(code.begin() + i,
                    new IntermediateCode(INSTR_LOADD, PLACEHOLDER, PLACEHOLDER, memory_map.at(roperand), labels));
                    code.insert(code.begin() + i + 1,
                    new IntermediateCode(instr, dest, loperand, DEST_TEMP, labels));
                    i++;
                    length++;
                }
                break;
            default:
                break;
        }
    }
}