#include "intermediate.h"
#include "../utils.h"
#include <stdio.h>

/**
 *  This function is similar to "generate_code_for_exp", BUT every expression
 *  will be regarded as a Right Value, including Variables, Index Expressions
 *  and Constant Arrays.
 * 
 *  This means that for Variables and Index Expressions, the returned value
 *  will be a COPY of the Variable, or the first element of the Index Expression,
 *  INSTEAD OF A REFERENCE. 
 * 
 *  A warning will be raised, if user tries to store the first element of an
 *  Index Expression or Constant Array with more than 1 elements.
 */
std::size_t IntermediateCodeGenerator::generate_code_for_exp_as_rval(Symbol* symbol)
{
    switch (symbol->symbol_idx)
    {
        case SYMBOL_VARIABLE:
        {
            auto addr = generate_code_for_exp(symbol);
            auto var = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_MRMOV, var, addr, PLACEHOLDER, statement_label
            ));
            return var;
        }
        case SYMBOL_NUMBER:
        {
            /** 
             *  When sizes.size() == 0, 'addr_or_var' is the register that stores
             *  the value;
             *  otherwise, 'addr_or_var' is the register that stores the address.
            */
            auto addr_or_var = generate_code_for_exp(symbol);
            if (((Number*)symbol)->sizes.size() > 0)
            {
                fprintf(stderr, "Semantic warning: try to load a value from type '%s'!\n",
                        array_type_to_str(((Number*)symbol)->sizes).c_str());
                auto var = generate_addr();
                code.push_back(new IntermediateCode(
                    INSTR_MRMOV, var, addr_or_var, PLACEHOLDER, statement_label
                ));
                return var;
            }
            return addr_or_var;
        }
        case SYMBOL_INDEX:
        {
            auto addr = generate_code_for_exp(symbol);
            if (symbol->type().array_lengths.size() > 0)
                fprintf(stderr, "Semantic warning: try to load a value from type '%s'!\n",
                        array_type_to_str(symbol->type().array_lengths).c_str());
            auto var = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_MRMOV, var, addr, PLACEHOLDER, statement_label
            ));
            return var;
        }
        default:
            return generate_code_for_exp(symbol);
    }
}

/**
 *  NOTICE: All registers must be assigned only ONCE!
 * 
 *  The semantics of this function is described below:
 *      For Left Values (including Variables and Indexing Expressions),
 *      the returned integer is the cardinal of register in which the
 *      ADDRESS of the Left Value is stored.
 * 
 *      For Right Values EXCEPT a pure number, the returned integer
 *      is the cardinal of register in which the VALUE is stored.
 * 
 *      For a pure number, since it can be a scalar or an array, we return
 *      the register containing its VALUE when it is a SCALAR, and
 *      the register containing its ADDRESS when it is an ARRAY.
 */
std::size_t IntermediateCodeGenerator::generate_code_for_exp(Symbol* symbol)
{
    switch (symbol->symbol_idx)
    { 
        case SYMBOL_VARIABLE:
        {
            Variable* as_variable = (Variable*)symbol;
            auto var = generate_addr();
            if (as_variable->entry->init_exp.size() > 0)
            {
                /* Non-empty 'init_exp' field, indicating a local variable.
                   In this case, the 'addr' field is the register storing
                   the address of the variable. */
                code.push_back(new IntermediateCode(
                    INSTR_RRMOV, var, as_variable->entry->addr, PLACEHOLDER, statement_label
                ));
                return var;
            }
            /* Otherwise, the 'init_val' field will be non-empty, indicating a
               global variable. In this case, the 'addr' field is the absolute
               address (i.e. an immediate). */
            code.push_back(new IntermediateCode(
                INSTR_IRMOV, var, as_variable->entry->addr, PLACEHOLDER, statement_label
            ));
            /** NOTICE: we won't be concerned with where to put the global variables
             *  and functions, since this is the assembler and the linker's 
             *  responsibility. 
             *  Concretely, the assembler will leave the "immediates" that should be 
             *  the address of global variables and functions as zero, but copy the 
             *  global symbol table we have produced to the objective file. The linker
             *  then reads the global symbol table, and choose the address of global
             *  variables and functions following hardware & OS dependent conventions.
             *  This is just "relocation".
             */
            return var; // a register that stores the address of the variable
        }       
        case SYMBOL_NUMBER:
        {
            Number* as_number = (Number*)symbol;
            auto var = generate_addr();

            if (as_number->sizes.size() == 0) /* Initialize a scalar */
            {
                /* Store the value directly in 'var'. */
                code.push_back(new IntermediateCode(
                    INSTR_IRMOV, var, (std::size_t)(as_number->value[0]), PLACEHOLDER, statement_label
                ));
            }
            else
            {
                /* Store a pointer to the value in 'var'. */
                auto size = as_number->value.size();
                code.push_back(new IntermediateCode(
                    INSTR_ALLOC, var, INT_SIZE * size, PLACEHOLDER, statement_label
                ));
                for (std::size_t i = 0; i < size; i++)
                {
                    auto init_val = generate_addr();
                    code.push_back(new IntermediateCode(
                        INSTR_IRMOV, init_val, (std::size_t)(as_number->value[i]), PLACEHOLDER, statement_label
                    ));
                    auto offset = generate_addr();
                    code.push_back(new IntermediateCode(
                        INSTR_IRMOV, offset, INT_SIZE * i, PLACEHOLDER, statement_label
                    ));
                    auto offset_addr = generate_addr();
                    code.push_back(new IntermediateCode(
                        INSTR_ADD, offset_addr, var, offset, statement_label
                    ));
                    code.push_back(new IntermediateCode(
                        INSTR_RMMOV, offset_addr, init_val, PLACEHOLDER , statement_label
                    ));
                }
            }
            return var;
        }
        case SYMBOL_UNARY:
        {
            UnaryExpression* as_unary = (UnaryExpression*)symbol;
            auto operand = generate_code_for_exp_as_rval(as_unary->children[0]);
            std::size_t var;
            switch (as_unary->operation)
            {
                case UNARY_EXP_OPERATOR_PLUS:
                    /* No need to generate an instruction for that. */
                    var = operand;
                    return var;
                case UNARY_EXP_OPERATOR_MINUS:
                    var = generate_addr();
                    code.push_back(new IntermediateCode(
                        INSTR_NEG, var, operand, PLACEHOLDER, statement_label
                    ));
                    return var;
                case UNARY_EXP_OPERATOR_NOT:
                    var = generate_addr();
                    code.push_back(new IntermediateCode(
                        INSTR_NOT, var, operand, PLACEHOLDER, statement_label
                    ));
                    return var;
            }
            return 0;
        }        
        case SYMBOL_ADD:
        {
            AddExpression* as_add = (AddExpression*)symbol;
            auto operand1 = generate_code_for_exp_as_rval(as_add->children[0]);
            auto operand2 = generate_code_for_exp_as_rval(as_add->children[1]);
            std::size_t var = generate_addr();
            switch (as_add->operation)
            {
                case ADD_EXP_OPERATOR_PLUS:
                    code.push_back(new IntermediateCode(
                        INSTR_ADD, var, operand1, operand2, statement_label
                    ));
                    return var;
                case ADD_EXP_OPERATOR_MINUS:
                    code.push_back(new IntermediateCode(
                        INSTR_SUB, var, operand1, operand2, statement_label
                    ));
                    return var;
            }
            return 0;
        }
        case SYMBOL_MUL:
        {
            MulExpression* as_mul = (MulExpression*)symbol;
            auto operand1 = generate_code_for_exp_as_rval(as_mul->children[0]);
            auto operand2 = generate_code_for_exp_as_rval(as_mul->children[1]);
            std::size_t var = generate_addr();
            switch (as_mul->operation)
            {
                case MUL_EXP_OPERATOR_TIMES:
                    code.push_back(new IntermediateCode(
                        INSTR_MUL, var, operand1, operand2, statement_label
                    ));
                    return var;
                case MUL_EXP_OPERATOR_DIVIDE:
                    code.push_back(new IntermediateCode(
                        INSTR_DIV, var, operand1, operand2, statement_label
                    ));
                    return var;
                case MUL_EXP_OPERATOR_MOD:
                    code.push_back(new IntermediateCode(
                        INSTR_MOD, var, operand1, operand2, statement_label
                    ));
                    return var;
            }
            return 0;
        }
        case SYMBOL_REL:
        {
            RelExpression* as_rel = (RelExpression*)symbol;
            auto operand1 = generate_code_for_exp_as_rval(as_rel->children[0]);
            auto operand2 = generate_code_for_exp_as_rval(as_rel->children[1]);
            std::size_t var = generate_addr();
            switch (as_rel->operation)
            {
                case REL_EXP_OPERATOR_G:
                    code.push_back(new IntermediateCode(
                        INSTR_GT, var, operand1, operand2, statement_label
                    ));
                    return var;
                case REL_EXP_OPERATOR_GEQ:
                    code.push_back(new IntermediateCode(
                        INSTR_GEQ, var, operand1, operand2, statement_label
                    ));
                    return var;
                case REL_EXP_OPERATOR_L:
                    code.push_back(new IntermediateCode(
                        INSTR_LT, var, operand1, operand2, statement_label
                    ));
                    return var;
                case REL_EXP_OPERATOR_LEQ:
                    code.push_back(new IntermediateCode(
                        INSTR_LEQ, var, operand1, operand2, statement_label
                    ));
                    return var;
            }
            return 0;
        }
        case SYMBOL_EQ:
        {
            EqExpression* as_eq = (EqExpression*)symbol;
            auto operand1 = generate_code_for_exp_as_rval(as_eq->children[0]);
            auto operand2 = generate_code_for_exp_as_rval(as_eq->children[1]);
            std::size_t var = generate_addr();
            switch (as_eq->operation)
            {
                case EQ_EXP_OPERATOR_EQ:
                    code.push_back(new IntermediateCode(
                        INSTR_EQ, var, operand1, operand2, statement_label
                    ));
                    return var;
                case EQ_EXP_OPERATOR_NEQ:
                    code.push_back(new IntermediateCode(
                        INSTR_NEQ, var, operand1, operand2, statement_label
                    ));
                    return var;
            }
            return 0;
        }
        case SYMBOL_INDEX:
        {
            /* For array indexing, we must return a reference to the array element,
               instead of creating a copy of the array. */
            IndexExpression* as_idx = (IndexExpression*)symbol;
            auto operand1 = generate_code_for_exp(as_idx->children[0]); 
            /* If the program passes semantic check, then 'operand1' must be a register
               that stores a memory address. */
            auto operand2 = generate_code_for_exp_as_rval(as_idx->children[1]);
            Type result_type = as_idx->type();

            std::size_t mul = 1;
            for (auto& len: result_type.array_lengths)
            {
                mul *= len;
            } /* Scale factor of the offset. */
            auto mov_size = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_IRMOV, mov_size, mul, PLACEHOLDER, statement_label
            ));
            auto offset = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_MUL, offset, mov_size, operand2, statement_label
            ));
            /* Return a register that stores the pointer to the array element. */
            auto var = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_ADD, var, operand1, offset, statement_label
            ));
            return var;
        }
        case SYMBOL_AND:
        {
            AndExpression* as_and = (AndExpression*)symbol;
            auto addr = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_ALLOC, addr, INT_SIZE, PLACEHOLDER, statement_label
            )); // must store result to memory since only single assignment
            auto operand1 = generate_code_for_exp_as_rval(as_and->children[0]);
            code.push_back(new IntermediateCode(
                INSTR_RMMOV, addr, operand1, PLACEHOLDER, statement_label
            ));

            auto next_instr = generate_label();
            code.push_back(new IntermediateCode(
                INSTR_JE, PLACEHOLDER, operand1, next_instr, statement_label
            )); // when 'operand1 == 0', jump to 'next_instr'

            auto operand2 = generate_code_for_exp_as_rval(as_and->children[1]);
            code.push_back(new IntermediateCode(
                INSTR_RMMOV, addr, operand2, PLACEHOLDER, statement_label
            ));
            statement_label = next_instr;
            auto var = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_MRMOV, var, addr, PLACEHOLDER, statement_label
            ));
            return var;
        }
        case SYMBOL_OR:
        {
            OrExpression* as_or = (OrExpression*)symbol;
            auto addr = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_ALLOC, addr, INT_SIZE, PLACEHOLDER, statement_label
            )); // must store result to memory since only single assignment
            auto operand1 = generate_code_for_exp_as_rval(as_or->children[0]);
            code.push_back(new IntermediateCode(
                INSTR_RMMOV, addr, operand1, PLACEHOLDER, statement_label
            ));

            auto next_instr = generate_label();
            code.push_back(new IntermediateCode(
                INSTR_JNE, PLACEHOLDER, operand1, next_instr, statement_label
            )); // when 'operand1 != 0', jump to 'next_instr'

            auto operand2 = generate_code_for_exp_as_rval(as_or->children[1]);
            code.push_back(new IntermediateCode(
                INSTR_RMMOV, addr, operand2, PLACEHOLDER, statement_label
            ));
            statement_label = next_instr;
            auto var = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_MRMOV, var, addr, PLACEHOLDER, statement_label
            ));
            return var;
        }
        case SYMBOL_FUNCTION:
        {
            Function* func_call = (Function*)symbol;
            auto size = func_call->children.size();
            for (std::size_t i = 0; i < size; i++)
            {
                auto arg_type = func_call->entry->type.ret_and_arg_types[i + 1];
                std::size_t arg;
                if (arg_type.array_lengths.size() == 0)
                {
                    /* Pass by value. */
                    arg = generate_code_for_exp_as_rval(func_call->children[i]);
                }
                else
                {
                    /* Pass by reference. */
                    arg = generate_code_for_exp(func_call->children[i]);
                }
                code.push_back(new IntermediateCode(
                    INSTR_ARG, PLACEHOLDER, i, arg, statement_label
                ));
            }
            auto var = generate_addr();
            code.push_back(new IntermediateCode(
                INSTR_CALL, var, func_call->entry->addr, size, statement_label
            ));
            return var;
        }
    }
    return 0;
}