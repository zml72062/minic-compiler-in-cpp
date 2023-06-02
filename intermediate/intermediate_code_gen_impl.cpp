#include "intermediate.h"
#include "../utils.h"
#include <stdio.h>

IntermediateCodeGenerator::IntermediateCodeGenerator()
{
    next_variable_label = 1;
    next_statement_label = 1;
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
