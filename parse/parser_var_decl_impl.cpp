#include "parser.h"
#include "symbols.h"
#include "../utils.h"
#include <stdio.h>

/**
 *  The first component records whether the parsing is successful;
 *  1 for true and 0 for false.
 *  The second component records the number of symbol table entries
 *  appended to the symbol table.
 */
std::pair<int, int> Parser::parse_next_var_decl()
{
    std::stack<int> var_decl_states;
    std::stack<void*> var_decl_symbols;
    var_decl_states.push(VAR_DECL_START);
    auto lexer_state = this->lexer.get_state();
    int n_entries = 0;

    while (true)
    {
        int can_parse = this->parse_var_decl_next_step(var_decl_states, var_decl_symbols);
        if (can_parse == -1)
        {
            this->lexer.restore_state(lexer_state);
            while(!var_decl_symbols.empty())
            {
                clear((Symbol*)var_decl_symbols.top());
                var_decl_symbols.pop();
            }
            for (auto i = 0; i < n_entries; i++)
                symbol_table->delete_entry();
            return std::pair<int, int>(0, 0);
        }
        if (can_parse == 1)
        {
            return std::pair<int, int>(1, n_entries);
        }
        if (can_parse == 2)
        {
            n_entries++;
        }
    }
}

int Parser::parse_var_decl_next_step(std::stack<int>& _states, 
                                     std::stack<void*>& _symbols)
{
    int state = _states.top();
    auto lexer_state = this->lexer.get_state();
    auto next_lexeme = this->lexer.next_lexeme();
    int next_val = next_lexeme.to_val();

    switch (state)
    {
        case VAR_DECL_START:
            if (next_val == INT_TYPE)
            {
                _states.push(VAR_DECL_GET_INT);
                return 0;
            }
            return -1;
        case VAR_DECL_GET_INT:
        case VAR_DECL_WAIT_FOR_ONE_MORE_DECL:
            if (next_val == IDENTIFIERS_VAL)
            {
                _states.push(VAR_DECL_GET_INT_IDENT);
                _symbols.push(new LexemePacker(next_lexeme));
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected identifier after "
                    "type 'int' at line %lu!\n", this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case VAR_DECL_GET_INT_IDENT:
            {
                /**
                 * Add the symbol to symbol table.
                 */
                LexemePacker* top_symbol = (LexemePacker*)(_symbols.top());
                
                /* Detect re-definition. */
                std::string name(top_symbol->lexeme.lex_name);
                if (symbol_table->get_entry_if_contains(name.c_str()) != nullptr)
                {
                    fprintf(stderr, "Semantic error: name '%s' "
                            "redefined at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }

                symbol_table->add_entry(
                    SymbolTableEntry(name, BASIC_TYPE_INT)
                );
            }
            _states.pop();
            _states.push(VAR_DECL_GET_VAR_NAME);
            this->lexer.restore_state(lexer_state);
            return 2;
        case VAR_DECL_GET_VAR_NAME:
            if (next_val == ASSIGN)
            {
                _states.push(VAR_DECL_WAIT_FOR_INIT_VAL);
                return 0;
            }
            if (next_val == LINDEX)
            {
                _states.push(VAR_DECL_WAIT_FOR_ARR_LEN);
                return 0;
            }
            _states.pop();
            {
                auto top_state = _states.top();
                if (top_state == VAR_DECL_GET_INT)
                {
                    _states.push(VAR_DECL_GET_FIRST_DECL);
                }
                else if (top_state == VAR_DECL_WAIT_FOR_ONE_MORE_DECL)
                {
                    _states.push(VAR_DECL_GET_ONE_MORE_DECL);
                }
                else
                    return -1;
            }
            {
                LexemePacker* top_symbol = (LexemePacker*)(_symbols.top());
                std::string name(top_symbol->lexeme.lex_name);
                auto entry = symbol_table->get_entry_if_contains(name.c_str());
                if (entry == nullptr)
                {
                    this->error = 1;
                    return -1;
                }

                auto& len = entry->type.array_lengths;
                std::size_t total_len = 1;
                for (auto& i: len)
                {
                    total_len *= i;
                }

                if (symbol_table->parent_scope() == nullptr) /* Global variables. */
                    entry->set_init_val(std::vector<int>(total_len, 0));
                else /* Local variables. Initialize to zero expression. */
                {
                    /**
                     *  NOTICE: Using an uninitialized local variable is undefined behavior
                     *  in SysY. Therefore, we can arbitrarily implement the initialization
                     *  of a local variable. Here we use zero initialization. 
                     */
                    std::vector<void*> init_exp(total_len);
                    for (auto i = 0; i < total_len; i++)
                    {
                        init_exp[i] = new Number(std::vector<std::size_t>(),
                                                 std::vector<int>({0}));
                    }
                    entry->set_init_exp(init_exp);
                }
                delete (Symbol*)top_symbol;
                _symbols.pop();
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case VAR_DECL_WAIT_FOR_INIT_VAL:
            if (next_val == LBRACE)
            {
                _states.push(VAR_DECL_WAIT_FOR_ARR_INIT_VAL);
                LexemePacker* ident_top = (LexemePacker*)(_symbols.top());
                std::string name(ident_top->lexeme.lex_name);
                auto entry = symbol_table->get_entry_if_contains(name.c_str());
                if (entry == nullptr)
                {
                    this->error = 1;
                    return -1;
                }
                auto& array_lengths = entry->type.array_lengths;
                if (array_lengths.size() == 0)
                {
                    fprintf(stderr, "Semantic error: expected a scalar value "
                            "for the initialization of variable '%s' with type 'int' "
                            "at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                if (symbol_table->parent_scope() == nullptr)
                    _symbols.push(new ArrayInitialization(array_lengths));
                else
                    _symbols.push(new ExpArrayInitialization(array_lengths));
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                LexemePacker* ident_top = (LexemePacker*)(_symbols.top());
                std::string name(ident_top->lexeme.lex_name);
                auto entry = symbol_table->get_entry_if_contains(name.c_str());
                if (entry == nullptr)
                {
                    this->error = 1;
                    return -1;
                }
                auto& array_lengths = entry->type.array_lengths;
                if (array_lengths.size() > 0)
                {
                    fprintf(stderr, "Semantic error: got a scalar value "
                            "for the initialization of array variable '%s' "
                            "at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                auto next_exp = this->parse_next_exp();
                if (next_exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                if (next_exp->symbol_idx != SYMBOL_NUMBER && symbol_table->parent_scope() == nullptr)
                {
                    /* Use a variable expression to initialize a global variable. */
                    fprintf(stderr, "Semantic error: expected a constant expression "
                            "but got a variable expression at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                _states.push(VAR_DECL_GET_SCALAR_INIT_VAL);
                if (symbol_table->parent_scope() == nullptr)
                    _symbols.push((Number*)(next_exp));
                else
                {
                    _symbols.push(new ExpArray(std::vector<std::size_t>(),
                                               std::vector<Symbol*>({next_exp})));
                }
                return 0;
            }
        case VAR_DECL_WAIT_FOR_ARR_LEN:
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
                _states.push(VAR_DECL_GET_ARR_LEN);
                _symbols.push((Number*)(next_exp));
                return 0;
            }
        case VAR_DECL_WAIT_FOR_ARR_INIT_VAL:
            if (next_val == LBRACE)
            {
                _states.push(VAR_DECL_WAIT_FOR_ARR_INIT_VAL);
                Symbol* top_init = (Symbol*)(_symbols.top());
                if (top_init->symbol_idx == SYMBOL_ARR_INIT)
                {
                    auto as_array_init = (ArrayInitialization*)top_init;
                    std::vector<std::size_t> new_sizes(as_array_init->sizes.begin() + 1, as_array_init->sizes.end());
                    if (new_sizes.size() == 0)
                    {
                        fprintf(stderr, "Semantic error: dimensionalities of initial value "
                                "and variable mismatch at line %lu!\n",
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    ArrayInitialization* sub_init = new ArrayInitialization(new_sizes);
                    as_array_init->add_subvalue(sub_init);
                    _symbols.push(sub_init);
                }
                else if (top_init->symbol_idx == SYMBOL_EXP_ARR_INIT)
                {
                    auto as_exp_array_init = (ExpArrayInitialization*)top_init;
                    std::vector<std::size_t> new_sizes(as_exp_array_init->sizes.begin() + 1, as_exp_array_init->sizes.end());
                    if (new_sizes.size() == 0)
                    {
                        fprintf(stderr, "Semantic error: dimensionalities of initial value "
                                "and variable mismatch at line %lu!\n",
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    ExpArrayInitialization* sub_init = new ExpArrayInitialization(new_sizes);
                    as_exp_array_init->add_subvalue(sub_init);
                    _symbols.push(sub_init);
                }
                else
                    return -1;
                return 0;
            }
            if (next_val == RBRACE)
            {
                _states.push(VAR_DECL_GET_EMPTY_ARR_INIT_VAL);
                Symbol* top_init = (Symbol*)(_symbols.top());
                if (top_init->symbol_idx == SYMBOL_ARR_INIT)
                {
                    ArrayInitialization* as_array_init = (ArrayInitialization*)top_init;
                    _symbols.pop();
                    if (as_array_init->parent == nullptr)
                    {
                        /* For top-level array initialization, must write value to (Number*) form. */
                        auto value = as_array_init->to_vector();
                        auto sizes = as_array_init->sizes;
                        _symbols.push(new Number(sizes, value));
                        clear(as_array_init);
                    }
                }
                else if (top_init->symbol_idx == SYMBOL_EXP_ARR_INIT)
                {
                    ExpArrayInitialization* as_exp_array_init = (ExpArrayInitialization*)top_init;
                    _symbols.pop();
                    if (as_exp_array_init->parent == nullptr)
                    {
                        /* For top-level array initialization, must write value to (Number*) form. */
                        auto value = as_exp_array_init->to_vector();
                        auto sizes = as_exp_array_init->sizes;
                        _symbols.push(new ExpArray(sizes, value));
                        clear_exp_arr_init(as_exp_array_init);
                    }
                }
                else
                    return -1;
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* top_init = (Symbol*)(_symbols.top());
                auto next_exp = this->parse_next_exp();
                if (next_exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }

                if (top_init->symbol_idx == SYMBOL_ARR_INIT)
                {
                    ArrayInitialization* as_array_init = (ArrayInitialization*)top_init;
                    if (next_exp->symbol_idx != SYMBOL_NUMBER)
                    {
                        fprintf(stderr, "Semantic error: expected a constant expression "
                                "but got a variable expression at line %lu!\n",
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_array_init->add_subvalue(next_exp);
                }
                else if (top_init->symbol_idx == SYMBOL_EXP_ARR_INIT)
                {
                    ExpArrayInitialization* as_exp_array_init = (ExpArrayInitialization*)top_init;
                    as_exp_array_init->add_subvalue(next_exp);
                }
                _states.push(VAR_DECL_GET_SCALAR_INIT_VAL);
                return 0;
            }
        case VAR_DECL_GET_ARR_LEN:
            if (next_val == RINDEX)
            {
                _states.push(VAR_DECL_GET_ARR_TYPE_SUFFIX);
                return 0;
            }
            return -1;
        case VAR_DECL_GET_ARR_TYPE_SUFFIX:
            {
                Number* arr_type_len = (Number*)(_symbols.top());
                if (arr_type_len->sizes.size() > 0)
                {
                    fprintf(stderr, "Semantic error: value of array type '%s' "
                            "cannot be the length of an array at line %lu!\n",
                            array_type_to_str(arr_type_len->sizes).c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;   
                }
                int arr_len = arr_type_len->value[0];
                if (arr_len < 0)
                {
                    fprintf(stderr, "Semantic error: value %d "
                            "cannot be the length of an array at line %lu!\n",
                            arr_len, this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                delete (Symbol*)(_symbols.top());
                _symbols.pop();

                LexemePacker* top_symbol = (LexemePacker*)(_symbols.top());
                std::string name(top_symbol->lexeme.lex_name);
                /* Update symbol table entry. */
                auto entry = symbol_table->get_entry_if_contains(name.c_str());
                if (entry != nullptr)
                    entry->type.create_array_type((std::size_t) arr_len);

                _states.pop();
                _states.pop();
                _states.pop();
                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case VAR_DECL_GET_NON_EMPTY_ARR_INIT_VAL:
            _states.pop();
        case VAR_DECL_GET_EMPTY_ARR_INIT_VAL:
            _states.pop();
        case VAR_DECL_GET_SCALAR_INIT_VAL:
            _states.pop();
            this->lexer.restore_state(lexer_state);
            switch (_states.top())
            {
                case VAR_DECL_WAIT_FOR_INIT_VAL:
                    _states.push(VAR_DECL_GET_ONE_VAR_DEF);
                    return 0;
                case VAR_DECL_WAIT_FOR_ARR_INIT_VAL:
                    _states.push(VAR_DECL_GET_FIRST_ARR_INIT_VAL);
                    return 0;
                case VAR_DECL_GET_MORE_INIT_VAL:
                    _states.push(VAR_DECL_GET_ONE_MORE_INIT_VAL);
                    return 0;
                default:
                    return 0;
            }
        case VAR_DECL_GET_MORE_INIT_VAL:
            if (next_val == LBRACE)
            {
                _states.push(VAR_DECL_WAIT_FOR_ARR_INIT_VAL);
                Symbol* top_init = (Symbol*)(_symbols.top());
                if (top_init->symbol_idx == SYMBOL_ARR_INIT)
                {
                    auto as_array_init = (ArrayInitialization*)top_init;
                    std::vector<std::size_t> new_sizes(as_array_init->sizes.begin() + 1, as_array_init->sizes.end());
                    if (new_sizes.size() == 0)
                    {
                        fprintf(stderr, "Semantic error: dimensionalities of initial value "
                                "and variable mismatch at line %lu!\n",
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    ArrayInitialization* sub_init = new ArrayInitialization(new_sizes);
                    as_array_init->add_subvalue(sub_init);
                    _symbols.push(sub_init);
                }
                else if (top_init->symbol_idx == SYMBOL_EXP_ARR_INIT)
                {
                    auto as_exp_array_init = (ExpArrayInitialization*)top_init;
                    std::vector<std::size_t> new_sizes(as_exp_array_init->sizes.begin() + 1, as_exp_array_init->sizes.end());
                    if (new_sizes.size() == 0)
                    {
                        fprintf(stderr, "Semantic error: dimensionalities of initial value "
                                "and variable mismatch at line %lu!\n",
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    ExpArrayInitialization* sub_init = new ExpArrayInitialization(new_sizes);
                    as_exp_array_init->add_subvalue(sub_init);
                    _symbols.push(sub_init);
                }
                else
                    return -1;
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* top_init = (Symbol*)(_symbols.top());
                auto next_exp = this->parse_next_exp();
                if (next_exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression "
                            "at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }

                if (top_init->symbol_idx == SYMBOL_ARR_INIT)
                {
                    ArrayInitialization* as_array_init = (ArrayInitialization*)top_init;
                    if (next_exp->symbol_idx != SYMBOL_NUMBER)
                    {
                        fprintf(stderr, "Semantic error: expected a constant expression "
                                "but got a variable expression at line %lu!\n",
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_array_init->add_subvalue(next_exp);
                }
                else if (top_init->symbol_idx == SYMBOL_EXP_ARR_INIT)
                {
                    ExpArrayInitialization* as_exp_array_init = (ExpArrayInitialization*)top_init;
                    as_exp_array_init->add_subvalue(next_exp);
                }
                _states.push(VAR_DECL_GET_SCALAR_INIT_VAL);
                return 0;
            }
        case VAR_DECL_SUCCEED:
            if (next_val == COMMA)
            {
                _states.push(VAR_DECL_WAIT_FOR_ONE_MORE_DECL);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            return 1;
        case VAR_DECL_GET_FIRST_DECL:
            _states.pop();
            _states.pop();
            _states.pop();
            _states.push(VAR_DECL_SUCCEED);
            this->lexer.restore_state(lexer_state);
            return 0;
        case VAR_DECL_GET_ONE_MORE_DECL:
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0; 
        case VAR_DECL_GET_FIRST_ARR_INIT_VAL:
            if (next_val == COMMA || next_val == RBRACE)
            {
                _states.pop();
                _states.push(VAR_DECL_WAIT_FOR_OPTIONALLY_MORE_INIT_VAL);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case VAR_DECL_WAIT_FOR_OPTIONALLY_MORE_INIT_VAL:
            if (next_val == COMMA)
            {
                _states.push(VAR_DECL_GET_MORE_INIT_VAL);
                return 0;
            }
            else if (next_val == RBRACE)
            {
                _states.push(VAR_DECL_GET_NON_EMPTY_ARR_INIT_VAL);
                Symbol* top_init = (Symbol*)(_symbols.top());
                if (top_init->symbol_idx == SYMBOL_ARR_INIT)
                {
                    ArrayInitialization* as_array_init = (ArrayInitialization*)top_init;
                    _symbols.pop();
                    if (as_array_init->parent == nullptr)
                    {
                        /* For top-level array initialization, must write value to (Number*) form. */
                        auto value = as_array_init->to_vector();
                        auto sizes = as_array_init->sizes;
                        _symbols.push(new Number(sizes, value));
                        clear(as_array_init);
                    }
                }
                else if (top_init->symbol_idx == SYMBOL_EXP_ARR_INIT)
                {
                    ExpArrayInitialization* as_exp_array_init = (ExpArrayInitialization*)top_init;
                    _symbols.pop();
                    if (as_exp_array_init->parent == nullptr)
                    {
                        /* For top-level array initialization, must write value to (Number*) form. */
                        auto value = as_exp_array_init->to_vector();
                        auto sizes = as_exp_array_init->sizes;
                        _symbols.push(new ExpArray(sizes, value));
                        clear_exp_arr_init(as_exp_array_init);
                    }
                }
                else
                    return -1;
                return 0;
            }
            return -1;
        case VAR_DECL_GET_ONE_MORE_INIT_VAL:
            if (next_val == COMMA || next_val == RBRACE)
            {
                _states.pop();
                _states.pop();
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case VAR_DECL_GET_ONE_VAR_DEF:
            {
                /* Get numerical value for the constant. */
                Symbol* top_symbol = (Symbol*)(_symbols.top());
                if (top_symbol->symbol_idx == SYMBOL_NUMBER)
                {
                    std::vector<int> value(((Number*)top_symbol)->value);
                    delete (Symbol*)(_symbols.top());
                    _symbols.pop();

                    LexemePacker* top_lexeme = (LexemePacker*)(_symbols.top());
                    auto entry = symbol_table->get_entry_if_contains(top_lexeme->lexeme.lex_name.c_str());
                    if (entry == nullptr)
                        return -1;
                    entry->set_init_val(value);
                    delete (Symbol*)(_symbols.top());
                    _symbols.pop();
                }
                else if (top_symbol->symbol_idx == SYMBOL_EXP_ARR)
                {
                    std::vector<Symbol*> value(((ExpArray*)top_symbol)->value);
                    delete (Symbol*)(_symbols.top());
                    _symbols.pop();
                    
                    LexemePacker* top_lexeme = (LexemePacker*)(_symbols.top());
                    auto entry = symbol_table->get_entry_if_contains(top_lexeme->lexeme.lex_name.c_str());
                    if (entry == nullptr)
                        return -1;
                    auto value_size = value.size();
                    std::vector<void*> value_cast(value_size);
                    for (auto i = 0; i < value_size; i++)
                        value_cast[i] = value[i];
                    entry->set_init_exp(value_cast);
                    delete (Symbol*)(_symbols.top());
                    _symbols.pop();
                }
                else 
                    return -1;
            }

            _states.pop();
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                if (top_state == VAR_DECL_WAIT_FOR_ONE_MORE_DECL)
                    _states.push(VAR_DECL_GET_ONE_MORE_DECL);
                else if (top_state == VAR_DECL_GET_INT)
                    _states.push(VAR_DECL_GET_FIRST_DECL);
                else
                    return -1;
            }
            this->lexer.restore_state(lexer_state);
            return 0;
    }
    return -1;
}
