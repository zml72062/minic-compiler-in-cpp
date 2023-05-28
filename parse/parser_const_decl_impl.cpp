#include "parser.h"
#include "symbols.h"
#include "../utils.h"
#include <stdio.h>

int Parser::parse_next_const_decl()
{
    std::stack<int> const_decl_states;
    std::stack<void*> const_decl_symbols;
    const_decl_states.push(CONST_DECL_START);
    auto lexer_state = this->lexer.get_state();

    while (true)
    {
        int can_parse = this->parse_const_decl_next_step(const_decl_states, const_decl_symbols);
        if (can_parse == -1)
        {
            this->lexer.restore_state(lexer_state);
            while(!const_decl_symbols.empty())
            {
                clear((Symbol*)const_decl_symbols.top());
                const_decl_symbols.pop();
            }
            return 0;
        }
        if (can_parse == 1)
        {
            return 1;
        }
    }
}

int Parser::parse_const_decl_next_step(std::stack<int>& _states, 
                                       std::stack<void*>& _symbols)
{
    int state = _states.top();
    auto lexer_state = this->lexer.get_state();
    auto next_lexeme = this->lexer.next_lexeme();
    int next_val = next_lexeme.to_val();

    switch (state)
    {
        case CONST_DECL_START:
            if (next_val == CONST_QUALIFIER)
            {
                _states.push(CONST_DECL_GET_CONST);
                return 0;
            }
            return -1;
        case CONST_DECL_GET_CONST:
            if (next_val == INT_TYPE)
            {
                _states.push(CONST_DECL_GET_CONST_INT);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected type 'int' after "
                    "qualifier 'const' at line %lu!\n", 
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case CONST_DECL_GET_CONST_INT:
        case CONST_DECL_WAIT_FOR_ONE_MORE_DECL:
            if (next_val == IDENTIFIERS_VAL)
            {
                _states.push(CONST_DECL_GET_CONST_INT_IDENT);
                _symbols.push(new LexemePacker(next_lexeme));
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected identifier after "
                    "type 'const int' at line %lu!\n", 
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case CONST_DECL_GET_CONST_INT_IDENT:
            if (next_val == ASSIGN || next_val == LINDEX)
            {
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
                        SymbolTableEntry(name, BASIC_TYPE_CONST_INT)
                    );
                }
                _states.pop();
                _states.push(CONST_DECL_GET_CONST_NAME);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
        case CONST_DECL_GET_CONST_NAME:
            if (next_val == ASSIGN)
            {
                _states.push(CONST_DECL_WAIT_FOR_INIT_VAL);
                return 0;
            }
            if (next_val == LINDEX)
            {
                _states.push(CONST_DECL_WAIT_FOR_ARR_LEN);
                return 0;
            }
            return -1;
        case CONST_DECL_WAIT_FOR_INIT_VAL:
            if (next_val == LBRACE)
            {
                _states.push(CONST_DECL_WAIT_FOR_ARR_INIT_VAL);
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
                            "for the initialization of variable '%s' with type 'const int' "
                            "at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                _symbols.push(new ArrayInitialization(array_lengths));
                return 0;
            }
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
                _symbols.top();
                _states.push(CONST_DECL_GET_SCALAR_INIT_VAL);
                _symbols.push((Number*)(next_exp));
                return 0;
            }
        case CONST_DECL_WAIT_FOR_ARR_LEN:
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
                _states.push(CONST_DECL_GET_ARR_LEN);
                _symbols.push((Number*)(next_exp));
                return 0;
            }
        case CONST_DECL_WAIT_FOR_ARR_INIT_VAL:
            if (next_val == LBRACE)
            {
                _states.push(CONST_DECL_WAIT_FOR_ARR_INIT_VAL);
                ArrayInitialization* top_init = (ArrayInitialization*)(_symbols.top());
                std::vector<std::size_t> new_sizes(top_init->sizes.begin() + 1, top_init->sizes.end());
                if (new_sizes.size() == 0)
                {
                    fprintf(stderr, "Semantic error: dimensionalities of initial value "
                            "and variable mismatch at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                ArrayInitialization* sub_init = new ArrayInitialization(new_sizes);
                top_init->add_subvalue(sub_init);
                _symbols.push(sub_init);
                return 0;
            }
            if (next_val == RBRACE)
            {
                _states.push(CONST_DECL_GET_EMPTY_ARR_INIT_VAL);
                ArrayInitialization* top_init = (ArrayInitialization*)(_symbols.top());
                _symbols.pop();
                if (top_init->parent == nullptr)
                {
                    /* For top-level array initialization, must write value to (Number*) form. */
                    auto value = top_init->to_vector();
                    auto sizes = top_init->sizes;
                    _symbols.push(new Number(sizes, value));
                    clear(top_init);
                }
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                ArrayInitialization* top_init = (ArrayInitialization*)(_symbols.top());
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
                _states.push(CONST_DECL_GET_SCALAR_INIT_VAL);
                top_init->add_subvalue(next_exp);
                return 0;
            }
        case CONST_DECL_GET_ARR_LEN:
            if (next_val == RINDEX)
            {
                _states.push(CONST_DECL_GET_ARR_TYPE_SUFFIX);
                return 0;
            }
            return -1;
        case CONST_DECL_GET_ARR_TYPE_SUFFIX:
            if (next_val == LINDEX || next_val == ASSIGN)
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
            return -1;
        case CONST_DECL_GET_MORE_INIT_VAL:
            if (next_val == LBRACE)
            {
                _states.push(CONST_DECL_WAIT_FOR_ARR_INIT_VAL);
                ArrayInitialization* top_init = (ArrayInitialization*)(_symbols.top());
                std::vector<std::size_t> new_sizes(top_init->sizes.begin() + 1, top_init->sizes.end());
                if (new_sizes.size() == 0)
                {
                    fprintf(stderr, "Semantic error: dimensionalities of initial value "
                            "and variable mismatch at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                ArrayInitialization* sub_init = new ArrayInitialization(new_sizes);
                top_init->add_subvalue(sub_init);
                _symbols.push(sub_init);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                ArrayInitialization* top_init = (ArrayInitialization*)(_symbols.top());
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
                _states.push(CONST_DECL_GET_SCALAR_INIT_VAL);
                top_init->add_subvalue(next_exp);
                return 0;
            }
        case CONST_DECL_SUCCEED:
            if (next_val == COMMA)
            {
                _states.push(CONST_DECL_WAIT_FOR_ONE_MORE_DECL);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            return 1;
        case CONST_DECL_GET_FIRST_DECL:
            _states.pop();
            _states.pop();
            _states.pop();
            _states.push(CONST_DECL_SUCCEED);
            this->lexer.restore_state(lexer_state);
            return 0;
        case CONST_DECL_GET_ONE_MORE_DECL:
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0; 
        case CONST_DECL_GET_NON_EMPTY_ARR_INIT_VAL:
            _states.pop();
        case CONST_DECL_GET_EMPTY_ARR_INIT_VAL:
            _states.pop();
        case CONST_DECL_GET_SCALAR_INIT_VAL:
            _states.pop();
            this->lexer.restore_state(lexer_state);
            switch (_states.top())
            {
                case CONST_DECL_WAIT_FOR_INIT_VAL:
                    _states.push(CONST_DECL_GET_ONE_CONST_DEF);
                    return 0;
                case CONST_DECL_WAIT_FOR_ARR_INIT_VAL:
                    _states.push(CONST_DECL_GET_FIRST_ARR_INIT_VAL);
                    return 0;
                case CONST_DECL_GET_MORE_INIT_VAL:
                    _states.push(CONST_DECL_GET_ONE_MORE_INIT_VAL);
                    return 0;
                default:
                    return 0;
            }
        case CONST_DECL_WAIT_FOR_OPTIONALLY_MORE_INIT_VAL:
            if (next_val == COMMA)
            {
                _states.push(CONST_DECL_GET_MORE_INIT_VAL);
                return 0;
            }
            else if (next_val == RBRACE)
            {
                _states.push(CONST_DECL_GET_NON_EMPTY_ARR_INIT_VAL);
                ArrayInitialization* top_init = (ArrayInitialization*)(_symbols.top());
                _symbols.pop();
                if (top_init->parent == nullptr)
                {
                    /* For top-level array initialization, must write value to (Number*) form. */
                    auto value = top_init->to_vector();
                    auto sizes = top_init->sizes;
                    _symbols.push(new Number(sizes, value));
                    clear(top_init);
                }
                return 0;
            }
            return -1;
        case CONST_DECL_GET_ONE_CONST_DEF:
            {
                /* Get numerical value for the constant. */
                Symbol* top_symbol = (Symbol*)(_symbols.top());
                if (top_symbol->symbol_idx != SYMBOL_NUMBER)
                    return -1;
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

            _states.pop();
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                if (top_state == CONST_DECL_WAIT_FOR_ONE_MORE_DECL)
                    _states.push(CONST_DECL_GET_ONE_MORE_DECL);
                else if (top_state == CONST_DECL_GET_CONST_INT)
                    _states.push(CONST_DECL_GET_FIRST_DECL);
                else
                    return -1;
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case CONST_DECL_GET_FIRST_ARR_INIT_VAL:
            if (next_val == COMMA || next_val == RBRACE)
            {
                _states.pop();
                _states.push(CONST_DECL_WAIT_FOR_OPTIONALLY_MORE_INIT_VAL);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;   
        case CONST_DECL_GET_ONE_MORE_INIT_VAL:
            if (next_val == COMMA || next_val == RBRACE)
            {
                _states.pop();
                _states.pop();
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            return -1;
    }
    return -1;
}
