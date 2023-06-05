#include "intermediate.h"
#include "../utils.h"
#include <stdio.h>
#include <map>

IntermediateCodeGenerator::IntermediateCodeGenerator()
{
    error = 0;
    next_variable_label = 1;
    next_statement_label = 1;
    next_global_symbol_label = (1 << 30) + 1;
    label_if_break = 0;
    label_if_continue = 0;
    statement_label = std::vector<std::size_t>();
}

IntermediateCodeGenerator::~IntermediateCodeGenerator()
{
    for (auto& code_line: code)
    {
        delete code_line;
    }
}

std::size_t IntermediateCodeGenerator::generate_addr()
{
    return next_variable_label++;
}

std::size_t IntermediateCodeGenerator::generate_label()
{
    return next_statement_label++;
}

std::size_t IntermediateCodeGenerator::generate_global_addr()
{
    return next_global_symbol_label++;
}

void IntermediateCodeGenerator::print_code()
{
    for (auto& code_line: code)
    {
        printf("%s\n", code_line->to_str().c_str());
    }
}

static bool is_global(std::size_t addr_or_label)
{
    return ((int)addr_or_label) > (1 << 30);
}

std::vector<IntermediateCode*> IntermediateCodeGenerator::simplify_code()
{
    auto length = code.size();
    /* Maps a target label to all possible jump instructions that point to it. */
    std::map<std::size_t, std::vector<std::size_t>> to_target;
    for (std::size_t i = 0; i < length; i++)
    {   
        auto code_line = code[i];
        if (code_line->instr == INSTR_JE ||
            code_line->instr == INSTR_JNE ||
            code_line->instr == INSTR_JMP)
        {
            try
            {
                to_target[code_line->roperand];
                to_target[code_line->roperand].push_back(i);
            }
            catch (const std::out_of_range& e)
            {
                to_target[code_line->roperand] = std::vector<std::size_t>({i});
            }
        }
        else if (code_line->instr == INSTR_CALL)
        {
            try
            {
                to_target[code_line->loperand];
                to_target[code_line->loperand].push_back(i);
            }
            catch (const std::out_of_range& e)
            {
                to_target[code_line->loperand] = std::vector<std::size_t>({i});
            }
        }
    }
    for (std::size_t i = 0; i < length; i++)
    {   
        auto code_line = code[i];
        if (code_line->labels.size() > 1) /* More than 1 labels. */
        {
            bool has_global = false;
            std::size_t chosen_label;
            for (auto& label: code_line->labels)
            {
                if (is_global(label))
                {
                    has_global = true;
                    chosen_label = label;
                }
            }
            if (!has_global)
            {
                chosen_label = code_line->labels[0];
            }

            /* Replace all occurrence of the multiple labels
               by the one chosen label. */
            for (auto& label: code_line->labels)
            {
                try
                {
                    /* The occurrence of label 'label' is listed
                       in to_target[label] */
                    for (auto& l: to_target[label])
                    {
                        if (code[l]->instr == INSTR_JE ||
                            code[l]->instr == INSTR_JNE ||
                            code[l]->instr == INSTR_JMP)
                        {
                            code[l]->roperand = chosen_label;
                        }
                        else if (code[l]->instr == INSTR_CALL)
                        {
                            code[l]->loperand = chosen_label;
                        }
                    }
                }
                catch(const std::out_of_range& e)
                {
                    
                }    
            }

            code_line->labels = std::vector<std::size_t>({chosen_label});
        }
    }

    std::vector<IntermediateCode*> copy;
    for (auto& code_line: code)
    {
        copy.push_back(new IntermediateCode(*code_line));
    }
    return copy;
}