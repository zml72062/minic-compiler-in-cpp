#include "intermediate.h"
#include "../utils.h"
#include <stdio.h>

void IntermediateCodeGenerator::generate_code()
{
    /* Go to top-level symbol table. */
    while (symbol_table->parent_scope() != nullptr)
    {
        symbol_table = symbol_table->parent_scope();
    }

    for (auto& entry: symbol_table->get_entries())
    {
        if (entry->type.basic_type == BASIC_TYPE_FUNC)
        {
            /****    Function definition.    ****/
            entry->addr = generate_global_addr();
            code.push_back(new IntermediateCode(
                INSTR_GLOB, PLACEHOLDER, entry->addr, PLACEHOLDER, statement_label
            ));
            /* Must guarantee that 'statement_label' is previously 0, see below. */
            statement_label.push_back(entry->addr);
            auto num_args = entry->type.ret_and_arg_types.size() - 1;
            /* The function arguments are recorded in this symbol table. 
               Must update entries in this table when translating. */
            auto args_scope = ((FunctionDef*)entry->func_def)->args_scope;
            for (std::size_t i = 0; i < num_args; i++)
            {
                auto var = generate_addr();
                code.push_back(new IntermediateCode(
                    INSTR_LARG, PLACEHOLDER, var, i, statement_label
                ));
                /* Set the address of the i-th argument to 'var'. */
                args_scope->get_entries()[i]->addr = var;
            }
            generate_code_for_block_and_statement(((FunctionDef*)entry->func_def)->children[0]);
            /* If control flow tries to move outside the function block, 
               we must stop it by adding return statements. */
            if (statement_label.size() > 0)
            {
                code.push_back(new IntermediateCode(
                    INSTR_RET, PLACEHOLDER, PLACEHOLDER, PLACEHOLDER, statement_label
                ));
            }
            /* Therefore, after every function definition, 'statement_label' must be 0.
               We don't need to worry about duplicate label at the beginning of a function. */
        }
        else if (entry->type.basic_type == BASIC_TYPE_INT)
        {
            /****    Global variable declaration.    ****/
            /* Following our convention, we store the address of a global 
               variable directly in the symbol table entry. */
            entry->addr = generate_global_addr();
            code.push_back(new IntermediateCode(
                INSTR_GLOB, PLACEHOLDER, entry->addr, PLACEHOLDER, statement_label
            ));
            /* Leave initialization to code generation stage. */
        }
    }
}