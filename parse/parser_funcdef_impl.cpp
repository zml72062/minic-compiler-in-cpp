#include "parser.h"
#include "symbols.h"
#include "../utils.h"
#include <stdio.h>

int Parser::parse_next_funcdef()
{
    std::stack<int> funcdef_states;
    std::stack<void*> funcdef_symbols;
    funcdef_states.push(FUNCDEF_START);
    auto lexer_state = this->lexer.get_state();

    while (true)
    {
        int can_parse = this->parse_funcdef_next_step(funcdef_states, funcdef_symbols);
        if (can_parse == -1)
        {
            this->lexer.restore_state(lexer_state);
            while(!funcdef_symbols.empty())
            {
                clear((Symbol*)funcdef_symbols.top());
                funcdef_symbols.pop();
            }
            return 0;
        }
        if (can_parse == 1)
        {
            return 1;
        }
    }
}

int Parser::parse_funcdef_next_step(std::stack<int>& _states, 
                                    std::stack<void*>& _symbols)
{
    int state = _states.top();
    auto lexer_state = this->lexer.get_state();
    auto next_lexeme = this->lexer.next_lexeme();
    int next_val = next_lexeme.to_val();

    switch (state)
    {
        case FUNCDEF_START:
            if (next_val == INT_TYPE)
            {
                _states.push(FUNCDEF_GET_INT);
                return 0;
            }
            if (next_val == VOID_TYPE)
            {
                _states.push(FUNCDEF_GET_VOID);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_INT:
            if (next_val == IDENTIFIERS_VAL)
            {
                _states.push(FUNCDEF_GET_INT_IDENT);
                _symbols.push(new LexemePacker(next_lexeme));
                return 0;
            }
            return -1;
        case FUNCDEF_GET_VOID:
            if (next_val == IDENTIFIERS_VAL)
            {
                _states.push(FUNCDEF_GET_VOID_IDENT);
                _symbols.push(new LexemePacker(next_lexeme));
                return 0;
            }
            return -1;
        case FUNCDEF_GET_INT_IDENT:
            if (next_val == LPAREN)
            {
                _states.push(FUNCDEF_GET_INT_LPAREN_WAIT_FOR_ARGS);

                LexemePacker* top_ident = (LexemePacker*)(_symbols.top());
                std::string name(top_ident->lexeme.lex_name);

                /* Detect redefinition. */
                if (symbol_table->get_entry_if_contains(name.c_str()))
                {
                    fprintf(stderr, "Semantic error: name '%s' "
                            "redefined at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                /* Add function name to symbol table. */
                symbol_table->add_entry(SymbolTableEntry(
                    name, BASIC_TYPE_FUNC
                ));
                /* Add return value type for the symbol table entry just registered. */
                auto entry = symbol_table->get_entry_if_contains(name.c_str());
                entry->type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));

                /* Create an intermediate symbol table
                   to store all the function arguments. */
                symbol_table = symbol_table->create_subscope();
                /* Top-level symbol table is accessed as 
                   symbol_table->parent_scope(). */

                _symbols.push(new FunctionDef(symbol_table));
                return 0;
            }
            return -1;
        case FUNCDEF_GET_VOID_IDENT:
            if (next_val == LPAREN)
            {
                _states.push(FUNCDEF_GET_VOID_LPAREN_WAIT_FOR_ARGS);

                LexemePacker* top_ident = (LexemePacker*)(_symbols.top());
                std::string name(top_ident->lexeme.lex_name);

                /* Detect redefinition. */
                if (symbol_table->get_entry_if_contains(name.c_str()))
                {
                    fprintf(stderr, "Semantic error: name '%s' "
                            "redefined at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                /* Add function name to symbol table. */
                symbol_table->add_entry(SymbolTableEntry(
                    name, BASIC_TYPE_FUNC
                ));
                /* Add return value type for the symbol table entry just registered. */
                auto entry = symbol_table->get_entry_if_contains(name.c_str());
                entry->type.add_ret_or_arg_type(Type(BASIC_TYPE_NONE));

                /* Create an intermediate symbol table
                   to store all the function arguments. */
                symbol_table = symbol_table->create_subscope();
                /* Top-level symbol table is accessed as 
                   symbol_table->parent_scope(). */

                _symbols.push(new FunctionDef(symbol_table));
                return 0;
            }
            return -1;
        case FUNCDEF_GET_INT_LPAREN_WAIT_FOR_ARGS:
            if (next_val == INT_TYPE)
            {
                _states.push(FUNCDEF_WAIT_FOR_AN_INT_ARGNAME);
                return 0;
            }
            if (next_val == RPAREN)
            {
                _states.push(FUNCDEF_GET_INT_WITHOUT_ARGS);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_VOID_LPAREN_WAIT_FOR_ARGS:
            if (next_val == INT_TYPE)
            {
                _states.push(FUNCDEF_WAIT_FOR_AN_INT_ARGNAME);
                return 0;
            }
            if (next_val == RPAREN)
            {
                _states.push(FUNCDEF_GET_VOID_WITHOUT_ARGS);
                return 0;
            }
            return -1;
        case FUNCDEF_WAIT_FOR_AN_INT_ARGNAME:
            if (next_val == IDENTIFIERS_VAL)
            {
                _states.push(FUNCDEF_GET_AN_INT_ARG);
                auto symbol_top = _symbols.top();
                _symbols.pop();
                /* Get the symbol table entry of the function. */
                auto entry = symbol_table->parent_scope()->get_entry_if_contains(
                    ((LexemePacker*)(_symbols.top()))->lexeme.lex_name.c_str()
                    );
                _symbols.push(symbol_top);
                _symbols.push(new LexemePacker(next_lexeme));

                if (entry == nullptr)
                    return -1;
                entry->type.add_ret_or_arg_type(Type(BASIC_TYPE_INT));
                symbol_table->add_entry(SymbolTableEntry(next_lexeme.lex_name, BASIC_TYPE_INT).local());
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected an identifier "
                    "at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case FUNCDEF_GET_INT_WITHOUT_ARGS:
            this->lexer.restore_state(lexer_state);
            {
                Block* block = (Block*)(this->parse_next_block());
                if (block == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected a block "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                symbol_table = symbol_table->parent_scope();

                FunctionDef* ptr_funcdef = (FunctionDef*)(_symbols.top());
                ptr_funcdef->add_definition(block);
                _symbols.pop();

                LexemePacker* top_lexeme = (LexemePacker*)(_symbols.top());
                auto entry = symbol_table->get_entry_if_contains(top_lexeme->lexeme.lex_name.c_str());
                if (entry == nullptr)
                    return -1;
                entry->set_func_def(ptr_funcdef);
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _states.push(FUNCDEF_GET_INT_FUNCDEF_WITHOUT_ARGS);
                return 0;
            }
        case FUNCDEF_GET_VOID_WITHOUT_ARGS:
            this->lexer.restore_state(lexer_state);
            {
                Block* block = (Block*)(this->parse_next_block());
                if (block == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected a block "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                symbol_table = symbol_table->parent_scope();

                FunctionDef* ptr_funcdef = (FunctionDef*)(_symbols.top());
                ptr_funcdef->add_definition(block);
                _symbols.pop();

                LexemePacker* top_lexeme = (LexemePacker*)(_symbols.top());
                auto entry = symbol_table->get_entry_if_contains(top_lexeme->lexeme.lex_name.c_str());
                if (entry == nullptr)
                    return -1;
                entry->set_func_def(ptr_funcdef);
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _states.push(FUNCDEF_GET_VOID_FUNCDEF_WITHOUT_ARGS);
                return 0;
            }
        case FUNCDEF_GET_AN_INT_ARG:
            if (next_val == LINDEX)
            {
                _states.push(FUNCDEF_WAIT_FOR_ARR_SUFFIX);
                return 0;
            }
            if (next_val == RPAREN || next_val == COMMA)
            {
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case FUNCDEF_GET_INT_LPAREN_WAIT_FOR_ARGS:
                    case FUNCDEF_GET_VOID_LPAREN_WAIT_FOR_ARGS:
                        _states.push(FUNCDEF_GET_FIRST_ARG);
                        break;
                    case FUNCDEF_WAIT_FOR_MORE_ARGS:
                        _states.push(FUNCDEF_GET_ONE_MORE_ARG);
                        break;
                    default:
                        return -1;
                }

                delete (Symbol*)(_symbols.top());
                _symbols.pop();

                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case FUNCDEF_WAIT_FOR_ARR_SUFFIX:
            if (next_val == RINDEX)
            {
                _states.push(FUNCDEF_GET_FIRST_ARR_SUFFIX);

                auto symbol_top1 = _symbols.top();
                _symbols.pop();
                auto symbol_top2 = _symbols.top();
                _symbols.pop();
                /* Get the symbol table entry of the function. */
                auto entry = symbol_table->parent_scope()->get_entry_if_contains(
                    ((LexemePacker*)(_symbols.top()))->lexeme.lex_name.c_str()
                    );
                _symbols.push(symbol_top2);
                _symbols.push(symbol_top1);

                if (entry == nullptr)
                    return -1;
                entry->type.ret_and_arg_types.back().create_array_type(0);
                auto entry_arg = symbol_table->get_entry_if_contains(
                    ((LexemePacker*)(_symbols.top()))->lexeme.lex_name.c_str());
                if (entry_arg == nullptr)
                    return -1;
                entry_arg->type.create_array_type(0);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ']' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case FUNCDEF_GET_FIRST_ARR_SUFFIX:
            if (next_val == LINDEX ||
                next_val == COMMA ||
                next_val == RPAREN)
            {
                _states.pop();
                _states.pop();
                _states.push(FUNCDEF_WAIT_FOR_OPTIONALLY_MORE_ARR_SUFFIX);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case FUNCDEF_WAIT_FOR_OPTIONALLY_MORE_ARR_SUFFIX:
            if (next_val == LINDEX)
            {
                _states.push(FUNCDEF_WAIT_FOR_NUMBERED_ARR_SUFFIX);
                return 0;
            }
            if (next_val == RPAREN || next_val == COMMA)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case FUNCDEF_GET_INT_LPAREN_WAIT_FOR_ARGS:
                    case FUNCDEF_GET_VOID_LPAREN_WAIT_FOR_ARGS:
                        _states.push(FUNCDEF_GET_FIRST_ARG);
                        break;
                    case FUNCDEF_WAIT_FOR_MORE_ARGS:
                        _states.push(FUNCDEF_GET_ONE_MORE_ARG);
                        break;
                    default:
                        return -1;
                }

                delete (Symbol*)(_symbols.top());
                _symbols.pop();

                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case FUNCDEF_WAIT_FOR_NUMBERED_ARR_SUFFIX:
            this->lexer.restore_state(lexer_state);
            {
                auto next_exp = this->parse_next_exp();
                if (next_exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                if (next_exp->symbol_idx != SYMBOL_NUMBER)
                {
                    fprintf(stderr, "Semantic error: expected a constant expression "
                            "but got a variable expression at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                Number* as_number = (Number*)next_exp;
                int value = as_number->value[0];
                delete as_number;

                _states.push(FUNCDEF_GET_ARR_LEN);

                auto symbol_top1 = _symbols.top();
                _symbols.pop();
                auto symbol_top2 = _symbols.top();
                _symbols.pop();
                /* Get the symbol table entry of the function. */
                auto entry = symbol_table->parent_scope()->get_entry_if_contains(
                    ((LexemePacker*)(_symbols.top()))->lexeme.lex_name.c_str()
                    );
                _symbols.push(symbol_top2);
                _symbols.push(symbol_top1);

                if (entry == nullptr)
                    return -1;
                entry->type.ret_and_arg_types.back().create_array_type((std::size_t)value);
                auto entry_arg = symbol_table->get_entry_if_contains(
                    ((LexemePacker*)(_symbols.top()))->lexeme.lex_name.c_str());
                if (entry_arg == nullptr)
                    return -1;
                entry_arg->type.create_array_type((std::size_t) value);
                return 0;
            }
        case FUNCDEF_GET_ARR_LEN:
            if (next_val == RINDEX)
            {
                _states.push(FUNCDEF_GET_A_NUMBERED_ARR_SUFFIX);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ']' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case FUNCDEF_GET_A_NUMBERED_ARR_SUFFIX:
            if (next_val == LINDEX ||
                next_val == COMMA ||
                next_val == RPAREN)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_FIRST_ARG:
            if (next_val == RPAREN || next_val == COMMA)
            {
                _states.pop();
                switch (_states.top())
                {
                    case FUNCDEF_GET_INT_LPAREN_WAIT_FOR_ARGS:
                        _states.push(FUNCDEF_GET_FIRST_ARG_WITH_RET_INT);
                        break;
                    case FUNCDEF_GET_VOID_LPAREN_WAIT_FOR_ARGS:
                        _states.push(FUNCDEF_GET_FIRST_ARG_WITH_RET_VOID);
                        break;
                    default:
                        return -1;
                }
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_FIRST_ARG_WITH_RET_INT:
            if (next_val == COMMA)
            {
                _states.push(FUNCDEF_WAIT_FOR_MORE_ARGS);
                return 0;
            }
            if (next_val == RPAREN)
            {
                _states.push(FUNCDEF_GET_INT_WITH_ARGS);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_FIRST_ARG_WITH_RET_VOID:
            if (next_val == COMMA)
            {
                _states.push(FUNCDEF_WAIT_FOR_MORE_ARGS);
                return 0;
            }
            if (next_val == RPAREN)
            {
                _states.push(FUNCDEF_GET_VOID_WITH_ARGS);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_INT_WITH_ARGS:
            this->lexer.restore_state(lexer_state);
            {
                Block* block = (Block*)(this->parse_next_block());
                if (block == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected a block "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                symbol_table = symbol_table->parent_scope();

                FunctionDef* ptr_funcdef = (FunctionDef*)(_symbols.top());
                ptr_funcdef->add_definition(block);
                _symbols.pop();

                LexemePacker* top_lexeme = (LexemePacker*)(_symbols.top());
                auto entry = symbol_table->get_entry_if_contains(top_lexeme->lexeme.lex_name.c_str());
                if (entry == nullptr)
                    return -1;
                entry->set_func_def(ptr_funcdef);
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _states.push(FUNCDEF_GET_INT_FUNCDEF_WITH_ARGS);
                return 0;
            }
        case FUNCDEF_GET_VOID_WITH_ARGS:
            this->lexer.restore_state(lexer_state);
            {
                Block* block = (Block*)(this->parse_next_block());
                if (block == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected a block "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                symbol_table = symbol_table->parent_scope();

                FunctionDef* ptr_funcdef = (FunctionDef*)(_symbols.top());
                ptr_funcdef->add_definition(block);
                _symbols.pop();

                LexemePacker* top_lexeme = (LexemePacker*)(_symbols.top());
                auto entry = symbol_table->get_entry_if_contains(top_lexeme->lexeme.lex_name.c_str());
                if (entry == nullptr)
                    return -1;
                entry->set_func_def(ptr_funcdef);
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _states.push(FUNCDEF_GET_VOID_FUNCDEF_WITH_ARGS);
                return 0;
            }
        case FUNCDEF_WAIT_FOR_MORE_ARGS:
            if (next_val == INT_TYPE)
            {
                _states.push(FUNCDEF_WAIT_FOR_AN_INT_ARGNAME);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_ONE_MORE_ARG:
            if (next_val == RPAREN || next_val == COMMA)
            {
                _states.pop();
                _states.pop();
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case FUNCDEF_GET_INT_FUNCDEF_WITH_ARGS:
        case FUNCDEF_GET_VOID_FUNCDEF_WITH_ARGS:
            _states.pop();
        case FUNCDEF_GET_INT_FUNCDEF_WITHOUT_ARGS:
        case FUNCDEF_GET_VOID_FUNCDEF_WITHOUT_ARGS:
            _states.pop();
            _states.pop();
            _states.pop();
            _states.pop();
            _states.pop();
            _states.push(FUNCDEF_SUCCEED);
            this->lexer.restore_state(lexer_state);
            return 0;
        case FUNCDEF_SUCCEED:
            this->lexer.restore_state(lexer_state);
            return 1;
    }
    return -1;
}
