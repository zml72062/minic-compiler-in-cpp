#include "intermediate.h"
#include "../utils.h"
#include <stdio.h>

IntermediateCodeGenerator::IntermediateCodeGenerator()
{
    error = 0;
    next_variable_label = 1;
    next_statement_label = 1;
    next_global_symbol_label = (1 << 30) + 1;
    label_if_break = 0;
    label_if_continue = 0;
    statement_label = 0;
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