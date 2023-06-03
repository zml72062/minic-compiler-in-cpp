#include "intermediate.h"
#include "../utils.h"
#include <stdio.h>

void IntermediateCodeGenerator::generate_code_for_block_and_statement(Symbol* symbol)
{
    switch (symbol->symbol_idx)
    {
        case SYMBOL_LOCAL_VAR_DECL:
        {
            LocalVarDeclaration* as_decl = (LocalVarDeclaration*)symbol;
            /* Following our convention, we store the address of a local 
               variable in a register, and record the cardinal of the 
               register in the symbol table entry. */
            auto var = generate_addr();
            as_decl->entry->addr = var;

            auto length = as_decl->entry->init_exp.size();
            code.push_back(new IntermediateCode(
                INSTR_ALLOC, var, INT_SIZE * length, PLACEHOLDER, statement_label
            ));
            for (std::size_t i = 0; i < length; i++)
            {
                /* Calculate initial value by evaluating expression. */
                auto init_val = generate_code_for_exp_as_rval((Symbol*)(as_decl->entry->init_exp[i]));
                auto offset = generate_addr();
                code.push_back(new IntermediateCode(
                    INSTR_IRMOV, offset, INT_SIZE * i, PLACEHOLDER, statement_label
                ));
                auto offset_addr = generate_addr();
                code.push_back(new IntermediateCode(
                    INSTR_ADD, offset_addr, var, offset, statement_label
                ));
                code.push_back(new IntermediateCode(
                    INSTR_RMMOV, offset_addr, init_val, PLACEHOLDER, statement_label
                ));
            }
            return;
        }
        case SYMBOL_EMPTY_STMT:
        /* Nothing has to be done. */
            return;
        case SYMBOL_ASSIGN_STMT:
        {
            AssignStatement* as_assign = (AssignStatement*)symbol;
            /* For LVal of the assignment, we need to generate the address. */
            auto lval_addr = generate_code_for_exp(as_assign->children[0]);
            /* For RVal of the assignment, we need to generate the value. */
            auto rval_addr = generate_code_for_exp_as_rval(as_assign->children[1]);
            code.push_back(new IntermediateCode(
                INSTR_RMMOV, lval_addr, rval_addr, PLACEHOLDER, statement_label
            ));
            return;
        }
        case SYMBOL_EXPR_STMT:
        {
            ExpressionStatement* as_exp = (ExpressionStatement*)symbol;
            /* Notice that even for 'void' functions, we formally return the value
               in a temporary register. */
            generate_code_for_exp(as_exp->children[0]);
            return;
        }
        case SYMBOL_IF_STMT:
        {
            IfStatement* as_if = (IfStatement*)symbol;
            auto expr = as_if->children[0];
            auto stmt = as_if->children[1];
            /* Generate code that evaluate the expression in 'expr_reg'. */
            auto expr_reg = generate_code_for_exp_as_rval(expr);

            auto if_not = generate_label(); /* Label to jump to if 'expr_reg == 0'. */
            code.push_back(new IntermediateCode(
                INSTR_JE, PLACEHOLDER, expr_reg, if_not, statement_label
            ));
            generate_code_for_block_and_statement(stmt);
            statement_label = if_not;
            return;
        }
        case SYMBOL_IF_ELSE_STMT:
        {
            IfElseStatement* as_if_else = (IfElseStatement*)symbol;
            auto expr = as_if_else->children[0];
            auto stmt_if = as_if_else->children[1];
            auto stmt_else = as_if_else->children[2];

            auto expr_reg = generate_code_for_exp_as_rval(expr);
            auto if_not = generate_label();
            code.push_back(new IntermediateCode(
                INSTR_JE, PLACEHOLDER, expr_reg, if_not, statement_label
            ));
            generate_code_for_block_and_statement(stmt_if);
            auto end_if = generate_label();
            code.push_back(new IntermediateCode(
                INSTR_JMP, PLACEHOLDER, PLACEHOLDER, end_if, statement_label
            ));
            statement_label = if_not;
            generate_code_for_block_and_statement(stmt_else);
            statement_label = end_if;
            return;
        }
        case SYMBOL_WHILE_STMT:
        {
            WhileStatement* as_while = (WhileStatement*)symbol;
            auto expr = as_while->children[0];
            auto stmt = as_while->children[1];
            /* Save 'break' and 'continue' labels, in case there are nested while-loops. */
            auto break_label = label_if_break;
            auto continue_label = label_if_continue;

            if (statement_label > 0)
            {
                /* There is already a label at the beginning. Then don't
                   generate a new label at the beginning. */
                label_if_continue = statement_label;
            }
            else
            {
                label_if_continue = generate_label();
                statement_label = label_if_continue;
            }
            auto expr_reg = generate_code_for_exp_as_rval(expr);
            label_if_break = generate_label();
            code.push_back(new IntermediateCode(
                INSTR_JE, PLACEHOLDER, expr_reg, label_if_break, statement_label
            ));
            generate_code_for_block_and_statement(stmt);
            code.push_back(new IntermediateCode(
                INSTR_JMP, PLACEHOLDER, PLACEHOLDER, label_if_continue, statement_label
            ));
            statement_label = label_if_break;

            /* Recover 'break' and 'continue' labels. */
            label_if_break = break_label;
            label_if_continue = continue_label;
            return;
        }
        case SYMBOL_BREAK_STMT:
            if (label_if_break == 0)
            {
                fprintf(stderr, "Semantic error: 'break' statement must be "
                        "inside a 'while' loop!\n");
                this->error = 1;
            }
            code.push_back(new IntermediateCode(
                INSTR_JMP, PLACEHOLDER, PLACEHOLDER, label_if_break, statement_label
            ));
            return;
        case SYMBOL_CONTINUE_STMT:
            if (label_if_continue == 0)
            {
                fprintf(stderr, "Semantic error: 'continue' statement must be "
                        "inside a 'while' loop!\n");
                this->error = 1;
            }
            code.push_back(new IntermediateCode(
                INSTR_JMP, PLACEHOLDER, PLACEHOLDER, label_if_continue, statement_label
            ));
            return;
        case SYMBOL_RETURN_NONE_STMT:
            code.push_back(new IntermediateCode(
                INSTR_RET, PLACEHOLDER, PLACEHOLDER, PLACEHOLDER, statement_label
            ));
            return;
        case SYMBOL_RETURN_VAL_STMT:
        {
            auto var = generate_code_for_exp_as_rval(symbol->children[0]);
            code.push_back(new IntermediateCode(
                INSTR_RET, PLACEHOLDER, var, PLACEHOLDER, statement_label
            ));
            return;
        }
        case SYMBOL_BLOCK:
            for (auto& stmt: symbol->children)
            {
                /* Here every 'stmt' can be one of the following:
                   EmptyStatement, ExpressionStatement, AssignmentStatement,
                   IfStatement, IfElseStatement, WhileStatement,
                   BreakStatement, ContinueStatement, ReturnNoneStatement,
                   ReturnValueStatement, Block, LocalVarDeclaration. */
                generate_code_for_block_and_statement(stmt);
            }
            return;
    }
}
