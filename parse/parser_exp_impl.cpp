#include "parser.h"
#include "symbols.h"
#include "../utils.h"
#include <stdio.h>

Symbol* Parser::parse_next_exp()
{
    std::stack<int> exp_states;
    std::stack<void*> exp_symbols;
    exp_states.push(EXP_START);
    auto lexer_state = this->lexer.get_state();

    while (true)
    {
        int can_parse = this->parse_exp_next_step(exp_states, exp_symbols);
        if (can_parse == -1)
        {
            this->lexer.restore_state(lexer_state);
            while(!exp_symbols.empty())
            {
                clear((Symbol*)exp_symbols.top());
                exp_symbols.pop();
            }
            return nullptr;
        }
        if (can_parse == 1)
            return (Symbol*)exp_symbols.top();
    }
}

/**
 *  Return 0, if possible to include one more token into an expression.
 *  Return 1, if succeed.
 *  Return -1, if impossible to parse.
 *  
 *  MUST manually recover state when return value is 1!!
 */
int Parser::parse_exp_next_step(std::stack<int>& _states,
                                std::stack<void*>& _symbols)
{
    int state = _states.top();
    auto lexer_state = this->lexer.get_state();
    auto next_lexeme = this->lexer.next_lexeme();
    int next_val = next_lexeme.to_val();

    switch (state)
    {
        /* We only pass "instances" (i.e. numbers, variables, functions) to stacks,
           instead of "notions" (i.e. left-values, expressions). */
        case EXP_GET_IDENT:
        {
            /**
             *  Check whether the identifier is in the symbol table.
             */
            LexemePacker* identifier = (LexemePacker*)(_symbols.top());
            const char* identifier_name = identifier->lexeme.lex_name.c_str();
            SymbolTableEntry* entry = symbol_table->get_entry_if_contains_in_tree(identifier_name);
            if (entry == nullptr)
            {
                fprintf(stderr, "Semantic error: identifier %s used without "
                        "definition at line %lu!\n", identifier_name,
                        this->lexer.get_lineno());
                this->error = 1;
                return -1;
            }
            int type_val = entry->type.basic_type;
            /**
             *  If we want to "call" the identity, we require it to be a function.
             */
            if (next_val == LPAREN)
            {
                if (type_val != BASIC_TYPE_FUNC ||
                    entry->type.array_lengths.size() > 0)
                {
                    fprintf(stderr, "Semantic error: identifier %s should be a "
                            "function at line %lu!\n", identifier_name,
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new Function(entry));
                _states.push(EXP_WANT_FUNC_CALL);
                return 0;
            }
            if (type_val == BASIC_TYPE_FUNC)
            {
                fprintf(stderr, "Semantic error: identifier %s should not be a "
                        "function at line %lu!\n", identifier_name,
                        this->lexer.get_lineno());
                this->error = 1;
                return -1;
            }
            if (type_val == BASIC_TYPE_INT)
            {
                /* A variable */
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new Variable(entry));
            }
            else if (type_val == BASIC_TYPE_CONST_INT)
            {
                /* Only keep its initial value */
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new Number(entry->type.array_lengths, 
                                         entry->init_val));
            }
            _states.pop();
            _states.push(EXP_GET_LVAL);
            this->lexer.restore_state(lexer_state);
            return 0;
        }
        case EXP_GET_NUMBER:
        {
            LexemePacker* number = (LexemePacker*)(_symbols.top());
            int num_val = number->lexeme.lex_value.value;
            delete (Symbol*)(_symbols.top());
            _symbols.pop();
            _symbols.push(new Number(std::vector<size_t>(), 
                                     std::vector<int>({num_val})));
            _states.pop();
            _states.push(EXP_GET_PRIMARY_EXP);
            this->lexer.restore_state(lexer_state);
            return 0;
        }
        case EXP_GET_LVAL:
            if (next_val == LINDEX)
            {
                _states.push(EXP_WANT_LVAL_INDEXING);
                return 0;
            }
            _states.pop();
            _states.push(EXP_GET_PRIMARY_EXP);
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_PRIMARY_EXP:
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_POSITIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_POSITIVE);
                        break;
                    case EXP_GET_NEGATIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NEGATIVE);
                        break;
                    case EXP_GET_NOT:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NOT);
                        break;
                    case EXP_GET_TIMES:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_TIMES);
                        break;
                    case EXP_GET_DIVIDE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_DIVIDE);
                        break;
                    case EXP_GET_MOD:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_MOD);
                        break;
                    default:
                        _states.push(EXP_GET_UNARY_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_FUNC_CALL_WITH_ARGS:
            {
                Function* func_top = (Function*)(_symbols.top());
                if (func_top->entry->type.ret_and_arg_types.size() !=
                    func_top->children.size() + 1)
                {
                    fprintf(stderr, "Semantic error: function %s got a wrong number of "
                            "arguments at line %lu!\n", func_top->entry->name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
            }
            _states.pop();
            _states.pop();
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_POSITIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_POSITIVE);
                        break;
                    case EXP_GET_NEGATIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NEGATIVE);
                        break;
                    case EXP_GET_NOT:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NOT);
                        break;
                    case EXP_GET_TIMES:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_TIMES);
                        break;
                    case EXP_GET_DIVIDE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_DIVIDE);
                        break;
                    case EXP_GET_MOD:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_MOD);
                        break;
                    default:
                        _states.push(EXP_GET_UNARY_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_FUNC_CALL_WITHOUT_ARGS:
            {
                Function* func_top = (Function*)(_symbols.top());
                if (func_top->entry->type.ret_and_arg_types.size() !=
                    func_top->children.size() + 1)
                {
                    fprintf(stderr, "Semantic error: function %s got a wrong number of "
                            "arguments at line %lu!\n", func_top->entry->name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
            }
            _states.pop();
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_POSITIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_POSITIVE);
                        break;
                    case EXP_GET_NEGATIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NEGATIVE);
                        break;
                    case EXP_GET_NOT:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NOT);
                        break;
                    case EXP_GET_TIMES:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_TIMES);
                        break;
                    case EXP_GET_DIVIDE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_DIVIDE);
                        break;
                    case EXP_GET_MOD:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_MOD);
                        break;
                    default:
                        _states.push(EXP_GET_UNARY_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP_AFTER_POSITIVE:
            {
                Symbol* copy_top_symbol = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                
                /** For numbers, we directly compute the value at 
                 *  compile time. */
                if (copy_top_symbol->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* top_symbol_as_a_number = (Number*)copy_top_symbol;
                    if (top_symbol_as_a_number->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value type '%s' is incompatible "
                                "with the operator '+' at line %lu!\n", 
                                array_type_to_str(top_symbol_as_a_number->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    _symbols.push(top_symbol_as_a_number);
                }
                else
                {
                    auto type = copy_top_symbol->type();
                    if (type.array_lengths.size() == 0 &&
                        (type.basic_type == BASIC_TYPE_CONST_INT ||
                        type.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new UnaryExpression(UNARY_EXP_OPERATOR_PLUS, copy_top_symbol));
                    }
                    else
                    {
                        if (type.basic_type == BASIC_TYPE_NONE)
                        {
                            fprintf(stderr, "Semantic error: value type 'void' is incompatible "
                                    "with the operator '+' at line %lu!\n", 
                                    this->lexer.get_lineno());
                            this->error = 1;
                            return -1;
                        }
                        if (type.basic_type == BASIC_TYPE_FUNC)
                        {
                            fprintf(stderr, "Semantic error: value type 'func' is incompatible "
                                    "with the operator '+' at line %lu!\n", 
                                    this->lexer.get_lineno());
                            this->error = 1;
                            return -1;
                        }
                        fprintf(stderr, "Semantic error: value type '%s' is incompatible "
                                "with the operator '+' at line %lu!\n", 
                                array_type_to_str(type.array_lengths).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_POSITIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_POSITIVE);
                        break;
                    case EXP_GET_NEGATIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NEGATIVE);
                        break;
                    case EXP_GET_NOT:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NOT);
                        break;
                    case EXP_GET_TIMES:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_TIMES);
                        break;
                    case EXP_GET_DIVIDE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_DIVIDE);
                        break;
                    case EXP_GET_MOD:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_MOD);
                        break;
                    default:
                        _states.push(EXP_GET_UNARY_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP_AFTER_NEGATIVE:
            {
                Symbol* copy_top_symbol = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                
                /** For numbers, we directly compute the value at 
                 *  compile time. */
                if (copy_top_symbol->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* top_symbol_as_a_number = (Number*)copy_top_symbol;
                    if (top_symbol_as_a_number->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value type '%s' is incompatible "
                                "with the operator '-' at line %lu!\n", 
                                array_type_to_str(top_symbol_as_a_number->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    top_symbol_as_a_number->value[0] = -(top_symbol_as_a_number->value[0]);
                    _symbols.push(top_symbol_as_a_number);
                }
                else
                {
                    auto type = copy_top_symbol->type();
                    if (type.array_lengths.size() == 0 &&
                        (type.basic_type == BASIC_TYPE_CONST_INT ||
                        type.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new UnaryExpression(UNARY_EXP_OPERATOR_MINUS, copy_top_symbol));
                    }
                    else
                    {
                        if (type.basic_type == BASIC_TYPE_NONE)
                        {
                            fprintf(stderr, "Semantic error: value type 'void' is incompatible "
                                    "with the operator '-' at line %lu!\n", 
                                    this->lexer.get_lineno());
                            this->error = 1;
                            return -1;
                        }
                        if (type.basic_type == BASIC_TYPE_FUNC)
                        {
                            fprintf(stderr, "Semantic error: value type 'func' is incompatible "
                                    "with the operator '-' at line %lu!\n", 
                                    this->lexer.get_lineno());
                            this->error = 1;
                            return -1;
                        }
                        fprintf(stderr, "Semantic error: value type '%s' is incompatible "
                                "with the operator '-' at line %lu!\n", 
                                array_type_to_str(type.array_lengths).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_POSITIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_POSITIVE);
                        break;
                    case EXP_GET_NEGATIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NEGATIVE);
                        break;
                    case EXP_GET_NOT:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NOT);
                        break;
                    case EXP_GET_TIMES:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_TIMES);
                        break;
                    case EXP_GET_DIVIDE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_DIVIDE);
                        break;
                    case EXP_GET_MOD:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_MOD);
                        break;
                    default:
                        _states.push(EXP_GET_UNARY_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP_AFTER_NOT:
            {
                Symbol* copy_top_symbol = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                
                /** For numbers, we directly compute the value at 
                 *  compile time. */
                if (copy_top_symbol->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* top_symbol_as_a_number = (Number*)copy_top_symbol;
                    if (top_symbol_as_a_number->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value type '%s' is incompatible "
                                "with the operator '!' at line %lu!\n", 
                                array_type_to_str(top_symbol_as_a_number->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    top_symbol_as_a_number->value[0] = !(top_symbol_as_a_number->value[0]);
                    _symbols.push(top_symbol_as_a_number);
                }
                else
                {
                    auto type = copy_top_symbol->type();
                    if (type.array_lengths.size() == 0 &&
                        (type.basic_type == BASIC_TYPE_CONST_INT ||
                        type.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new UnaryExpression(UNARY_EXP_OPERATOR_NOT, copy_top_symbol));
                    }
                    else
                    {
                        if (type.basic_type == BASIC_TYPE_NONE)
                        {
                            fprintf(stderr, "Semantic error: value type 'void' is incompatible "
                                    "with the operator '!' at line %lu!\n", 
                                    this->lexer.get_lineno());
                            this->error = 1;
                            return -1;
                        }
                        if (type.basic_type == BASIC_TYPE_FUNC)
                        {
                            fprintf(stderr, "Semantic error: value type 'func' is incompatible "
                                    "with the operator '!' at line %lu!\n", 
                                    this->lexer.get_lineno());
                            this->error = 1;
                            return -1;
                        }
                        fprintf(stderr, "Semantic error: value type '%s' is incompatible "
                                "with the operator '!' at line %lu!\n", 
                                array_type_to_str(type.array_lengths).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_POSITIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_POSITIVE);
                        break;
                    case EXP_GET_NEGATIVE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NEGATIVE);
                        break;
                    case EXP_GET_NOT:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_NOT);
                        break;
                    case EXP_GET_TIMES:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_TIMES);
                        break;
                    case EXP_GET_DIVIDE:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_DIVIDE);
                        break;
                    case EXP_GET_MOD:
                        _states.push(EXP_GET_UNARY_EXP_AFTER_MOD);
                        break;
                    default:
                        _states.push(EXP_GET_UNARY_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP:
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_PLUS:
                        _states.push(EXP_GET_MUL_EXP_AFTER_ADD);
                        break;
                    case EXP_GET_MINUS:
                        _states.push(EXP_GET_MUL_EXP_AFTER_MINUS);
                        break;
                    default:
                        _states.push(EXP_GET_MUL_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP_AFTER_TIMES:
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '*' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] *= (as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new MulExpression(MUL_EXP_OPERATOR_TIMES,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '*' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP_AFTER_DIVIDE:
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '/' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    if (as_number_r->value[0] == 0)
                    {
                        fprintf(stderr, "Semantic error: divide by zero at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] /= (as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new MulExpression(MUL_EXP_OPERATOR_DIVIDE,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '/' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_UNARY_EXP_AFTER_MOD:
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '%%' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    if (as_number_r->value[0] == 0)
                    {
                        fprintf(stderr, "Semantic error: divide by zero at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] %= (as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new MulExpression(MUL_EXP_OPERATOR_MOD,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '%%' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_MUL_EXP:
            if (next_val == TIMES_OPERATOR)
            {
                _states.push(EXP_GET_TIMES);
                return 0;
            }
            if (next_val == DIVIDE_OPERATOR)
            {
                _states.push(EXP_GET_DIVIDE);
                return 0;
            }
            if (next_val == MOD_OPERATOR)
            {
                _states.push(EXP_GET_MOD);
                return 0;
            }

            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_G:
                        _states.push(EXP_GET_ADD_EXP_AFTER_G);
                        break;
                    case EXP_GET_GEQ:
                        _states.push(EXP_GET_ADD_EXP_AFTER_GEQ);
                        break;
                    case EXP_GET_L:
                        _states.push(EXP_GET_ADD_EXP_AFTER_L);
                        break;
                    case EXP_GET_LEQ:
                        _states.push(EXP_GET_ADD_EXP_AFTER_LEQ);
                        break;
                    default:
                        _states.push(EXP_GET_ADD_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_MUL_EXP_AFTER_ADD:
            if (next_val == TIMES_OPERATOR)
            {
                _states.push(EXP_GET_TIMES);
                return 0;
            }
            if (next_val == DIVIDE_OPERATOR)
            {
                _states.push(EXP_GET_DIVIDE);
                return 0;
            }
            if (next_val == MOD_OPERATOR)
            {
                _states.push(EXP_GET_MOD);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '+' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] += (as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new AddExpression(ADD_EXP_OPERATOR_PLUS,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '+' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }

                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_MUL_EXP_AFTER_MINUS:
            if (next_val == TIMES_OPERATOR)
            {
                _states.push(EXP_GET_TIMES);
                return 0;
            }
            if (next_val == DIVIDE_OPERATOR)
            {
                _states.push(EXP_GET_DIVIDE);
                return 0;
            }
            if (next_val == MOD_OPERATOR)
            {
                _states.push(EXP_GET_MOD);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '-' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] -= (as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new AddExpression(ADD_EXP_OPERATOR_MINUS,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '-' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_ADD_EXP:
            if (next_val == PLUS_OPERATOR)
            {
                _states.push(EXP_GET_PLUS);
                return 0;
            }
            if (next_val == MINUS_OPERATOR)
            {
                _states.push(EXP_GET_MINUS);
                return 0;
            }
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_EQ:
                        _states.push(EXP_GET_REL_EXP_AFTER_EQ);
                        break;
                    case EXP_GET_NEQ:
                        _states.push(EXP_GET_REL_EXP_AFTER_NEQ);
                        break;
                    default:
                        _states.push(EXP_GET_REL_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_ADD_EXP_AFTER_G:
            if (next_val == PLUS_OPERATOR)
            {
                _states.push(EXP_GET_PLUS);
                return 0;
            }
            if (next_val == MINUS_OPERATOR)
            {
                _states.push(EXP_GET_MINUS);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '>' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] > as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new RelExpression(REL_EXP_OPERATOR_G,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '>' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_ADD_EXP_AFTER_GEQ:
            if (next_val == PLUS_OPERATOR)
            {
                _states.push(EXP_GET_PLUS);
                return 0;
            }
            if (next_val == MINUS_OPERATOR)
            {
                _states.push(EXP_GET_MINUS);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '>=' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] >= as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new RelExpression(REL_EXP_OPERATOR_GEQ,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '>=' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_ADD_EXP_AFTER_L:
            if (next_val == PLUS_OPERATOR)
            {
                _states.push(EXP_GET_PLUS);
                return 0;
            }
            if (next_val == MINUS_OPERATOR)
            {
                _states.push(EXP_GET_MINUS);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '<' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] < as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new RelExpression(REL_EXP_OPERATOR_L,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '<' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_ADD_EXP_AFTER_LEQ:
            if (next_val == PLUS_OPERATOR)
            {
                _states.push(EXP_GET_PLUS);
                return 0;
            }
            if (next_val == MINUS_OPERATOR)
            {
                _states.push(EXP_GET_MINUS);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '<=' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] <= as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new RelExpression(REL_EXP_OPERATOR_LEQ,
                                                        copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '<=' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_REL_EXP:
            if (next_val == G_OPERATOR)
            {
                _states.push(EXP_GET_G);
                return 0;
            }
            if (next_val == GEQ_OPERATOR)
            {
                _states.push(EXP_GET_GEQ);
                return 0;
            }
            if (next_val == L_OPERATOR)
            {
                _states.push(EXP_GET_L);
                return 0;
            }
            if (next_val == LEQ_OPERATOR)
            {
                _states.push(EXP_GET_LEQ);
                return 0;
            }
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_AND:
                        _states.push(EXP_GET_EQ_EXP_AFTER_AND);
                        break;
                    default:
                        _states.push(EXP_GET_EQ_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_REL_EXP_AFTER_EQ:
            if (next_val == G_OPERATOR)
            {
                _states.push(EXP_GET_G);
                return 0;
            }
            if (next_val == GEQ_OPERATOR)
            {
                _states.push(EXP_GET_GEQ);
                return 0;
            }
            if (next_val == L_OPERATOR)
            {
                _states.push(EXP_GET_L);
                return 0;
            }
            if (next_val == LEQ_OPERATOR)
            {
                _states.push(EXP_GET_LEQ);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '==' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] == as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new EqExpression(EQ_EXP_OPERATOR_EQ,
                                                    copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '==' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_REL_EXP_AFTER_NEQ:
            if (next_val == G_OPERATOR)
            {
                _states.push(EXP_GET_G);
                return 0;
            }
            if (next_val == GEQ_OPERATOR)
            {
                _states.push(EXP_GET_GEQ);
                return 0;
            }
            if (next_val == L_OPERATOR)
            {
                _states.push(EXP_GET_L);
                return 0;
            }
            if (next_val == LEQ_OPERATOR)
            {
                _states.push(EXP_GET_LEQ);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '!=' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] != as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new EqExpression(EQ_EXP_OPERATOR_NEQ,
                                                    copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '!=' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_EQ_EXP:
            if (next_val == EQ_OPERATOR)
            {
                _states.push(EXP_GET_EQ);
                return 0;
            }
            if (next_val == NEQ_OPERATOR)
            {
                _states.push(EXP_GET_NEQ);
                return 0;
            }
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_GET_OR:
                        _states.push(EXP_GET_AND_EXP_AFTER_OR);
                        break;
                    default:
                        _states.push(EXP_GET_AND_EXP);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_EQ_EXP_AFTER_AND:
            if (next_val == EQ_OPERATOR)
            {
                _states.push(EXP_GET_EQ);
                return 0;
            }
            if (next_val == NEQ_OPERATOR)
            {
                _states.push(EXP_GET_NEQ);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '&&' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] && as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new AndExpression(copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '&&' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_AND_EXP:
            if (next_val == AND_OPERATOR)
            {
                _states.push(EXP_GET_AND);
                return 0;
            }
            _states.pop();
            _states.push(EXP_GET_OR_EXP);
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_AND_EXP_AFTER_OR:
            if (next_val == AND_OPERATOR)
            {
                _states.push(EXP_GET_AND);
                return 0;
            }
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() > 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '||' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    as_number_l->value[0] = (as_number_l->value[0] || as_number_r->value[0]);
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() == 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new OrExpression(copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '||' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_OR_EXP:
            if (next_val == OR_OPERATOR)
            {
                _states.push(EXP_GET_OR);
                return 0;
            }
            _states.pop();
            {
                int top_state = _states.top();
                switch (top_state)
                {
                    case EXP_START:
                        _states.push(EXP_SUCCEED);
                        break;
                    case EXP_GET_LPAREN:
                        _states.push(EXP_GET_HALF_BRACKETED_EXP);
                        break;
                    case EXP_WANT_LVAL_INDEXING:
                        _states.push(EXP_GET_HALF_LVAL_INDEXING_EXP);
                        break;
                    case EXP_WANT_FUNC_CALL:
                        _states.push(EXP_GET_ONE_ARG);
                        break;
                    case EXP_WANT_ADDITIONAL_ARGS:
                        _states.push(EXP_GET_ONE_MORE_ARG);
                        break;
                }
            }
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_BRACKETED_EXP:
            _states.pop();
            _states.pop();
            _states.pop();
            _states.push(EXP_GET_PRIMARY_EXP);
            this->lexer.restore_state(lexer_state);
            return 0;
        case EXP_GET_LVAL_INDEXING_EXP:
            {
                Symbol* copy_top_symbol_r = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* copy_top_symbol_l = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                if (copy_top_symbol_l->symbol_idx == SYMBOL_NUMBER &&
                    copy_top_symbol_r->symbol_idx == SYMBOL_NUMBER)
                {
                    Number* as_number_l = (Number*)copy_top_symbol_l;
                    Number* as_number_r = (Number*)copy_top_symbol_r;
                    if (as_number_l->sizes.size() == 0 ||
                        as_number_r->sizes.size() > 0)
                    {
                        fprintf(stderr, "Semantic error: value types '%s' and '%s' are incompatible "
                                "with the operator '[]' at line %lu!\n", 
                                array_type_to_str(as_number_l->sizes).c_str(),
                                array_type_to_str(as_number_r->sizes).c_str(),
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                    int idx = as_number_r->value[0];
                    auto new_size = std::vector<std::size_t>(as_number_l->sizes.begin() + 1, as_number_l->sizes.end());
                    as_number_l->sizes = new_size;
                    
                    std::size_t multiplier = 1;
                    for (auto& size: new_size)
                    {
                        multiplier *= size;
                    }
                    auto new_value = std::vector<int>(as_number_l->value.begin() + idx * multiplier,
                                                      as_number_l->value.begin() + (idx + 1) * multiplier);
                    as_number_l->value = new_value;
                    delete as_number_r;
                    _symbols.push(as_number_l);
                }
                else
                {
                    auto type_l = copy_top_symbol_l->type();
                    auto type_r = copy_top_symbol_r->type();
                    if (type_l.array_lengths.size() > 0 &&
                        (type_l.basic_type == BASIC_TYPE_CONST_INT ||
                        type_l.basic_type == BASIC_TYPE_INT) &&
                        type_r.array_lengths.size() == 0 &&
                        (type_r.basic_type == BASIC_TYPE_CONST_INT ||
                        type_r.basic_type == BASIC_TYPE_INT))
                    {
                        _symbols.push(new IndexExpression(copy_top_symbol_l, copy_top_symbol_r));
                    }
                    else
                    {
                        fprintf(stderr, "Semantic error: value types are incompatible "
                                "with the operator '[]' at line %lu!\n", 
                                this->lexer.get_lineno());
                        this->error = 1;
                        return -1;
                    }
                }
            }
            _states.pop();
            _states.pop();
            _states.pop();
            this->lexer.restore_state(lexer_state);
            return 0; 
        case EXP_GET_HALF_BRACKETED_EXP:
            if (next_val == RPAREN)
            {
                _states.push(EXP_GET_BRACKETED_EXP);
                return 0;
            }
            return -1;
        case EXP_GET_HALF_LVAL_INDEXING_EXP:
            if (next_val == RINDEX)
            {
                _states.push(EXP_GET_LVAL_INDEXING_EXP);
                return 0;
            }
            return -1;
        case EXP_GET_ONE_ARG:
            if (next_val == COMMA ||
                next_val == RPAREN)
            {
                Symbol* copy_top_arg = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Function* copy_top_func = (Function*)(_symbols.top());

                auto index_arg = copy_top_func->children.size() + 1;
                auto rtype = copy_top_arg->type();
                if (index_arg >= copy_top_func->entry->type.ret_and_arg_types.size())
                {
                    fprintf(stderr, "Semantic error: function %s got a wrong number of "
                            "arguments at line %lu!\n", copy_top_func->entry->name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1; 
                }
                auto dtype = copy_top_func->entry->type.ret_and_arg_types[index_arg];
                if (dtype.array_lengths.size() == rtype.array_lengths.size() &&
                    (dtype.array_lengths.size() == 0 ||
                    std::vector<std::size_t>(dtype.array_lengths.begin() + 1,
                                             dtype.array_lengths.end()) == 
                    std::vector<std::size_t>(rtype.array_lengths.begin() + 1,
                                             rtype.array_lengths.end())) &&                      
                    (rtype.basic_type == BASIC_TYPE_INT || 
                    rtype.basic_type == BASIC_TYPE_CONST_INT))
                {
                    copy_top_func->add_argument(copy_top_arg);
                }
                else
                {
                    fprintf(stderr, "Semantic error: incompatible argument type "
                            "for the function call at line %lu!\n", 
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }

                _states.pop();
                _states.push(EXP_WANT_OPTIONALLY_MORE_ARGS);
                this->lexer.restore_state(lexer_state);
                return 0; 
            }
            return -1;
        case EXP_WANT_OPTIONALLY_MORE_ARGS:
            if (next_val == RPAREN)
            {
                _states.push(EXP_GET_FUNC_CALL_WITH_ARGS);
                return 0;
            }
            if (next_val == COMMA)
            {
                _states.push(EXP_WANT_ADDITIONAL_ARGS);
                return 0;
            }
            return -1;
        case EXP_GET_ONE_MORE_ARG:  
            if (next_val == COMMA ||
                next_val == RPAREN)
            {
                Symbol* copy_top_arg = ((Symbol*)(_symbols.top()))->copy();
                delete (Symbol*)(_symbols.top());
                _symbols.pop();
                Function* copy_top_func = (Function*)(_symbols.top());

                auto index_arg = copy_top_func->children.size() + 1;
                auto rtype = copy_top_arg->type();
                if (index_arg >= copy_top_func->entry->type.ret_and_arg_types.size())
                {
                    fprintf(stderr, "Semantic error: function %s got a wrong number of "
                            "arguments at line %lu!\n", copy_top_func->entry->name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1; 
                }
                auto dtype = copy_top_func->entry->type.ret_and_arg_types[index_arg];
                if (dtype.array_lengths.size() == rtype.array_lengths.size() &&
                    (dtype.array_lengths.size() == 0 ||
                    std::vector<std::size_t>(dtype.array_lengths.begin() + 1,
                                             dtype.array_lengths.end()) == 
                    std::vector<std::size_t>(rtype.array_lengths.begin() + 1,
                                             rtype.array_lengths.end())) &&                      
                    (rtype.basic_type == BASIC_TYPE_INT || 
                    rtype.basic_type == BASIC_TYPE_CONST_INT))
                {
                    copy_top_func->add_argument(copy_top_arg);
                }
                else
                {
                    fprintf(stderr, "Semantic error: incompatible argument type "
                            "for the function call at line %lu!\n", 
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }

                _states.pop();
                _states.pop();
                this->lexer.restore_state(lexer_state);
                return 0; 
            }
            return -1;
        case EXP_WANT_FUNC_CALL:
            if (next_val == RPAREN)
            {
                _states.push(EXP_GET_FUNC_CALL_WITHOUT_ARGS);
                return 0;
            }
        case EXP_START:
        case EXP_GET_LPAREN:
        case EXP_GET_POSITIVE:
        case EXP_GET_NEGATIVE:
        case EXP_GET_NOT:
        case EXP_WANT_LVAL_INDEXING:
        case EXP_GET_OR:
        case EXP_GET_TIMES:
        case EXP_GET_DIVIDE:
        case EXP_GET_MOD:
        case EXP_GET_PLUS:
        case EXP_GET_MINUS:
        case EXP_GET_G:
        case EXP_GET_GEQ:
        case EXP_GET_L:
        case EXP_GET_LEQ:
        case EXP_GET_EQ:
        case EXP_GET_NEQ:
        case EXP_GET_AND:
        case EXP_WANT_ADDITIONAL_ARGS:
            if (next_val == IDENTIFIERS_VAL)
            {
                _states.push(EXP_GET_IDENT);
                _symbols.push(new LexemePacker(next_lexeme));
                return 0;
            }
            if (next_val == LPAREN)
            {
                _states.push(EXP_GET_LPAREN);
                return 0;
            }
            if (next_val == NUMBERS_VAL)
            {
                _states.push(EXP_GET_NUMBER);
                _symbols.push(new LexemePacker(next_lexeme));
                return 0;
            }
            if (next_val == PLUS_OPERATOR)
            {
                _states.push(EXP_GET_POSITIVE);
                return 0;
            }
            if (next_val == MINUS_OPERATOR)
            {
                _states.push(EXP_GET_NEGATIVE);
                return 0;
            }
            if (next_val == NOT_OPERATOR)
            {
                _states.push(EXP_GET_NOT);
                return 0;
            }
            return -1;
        case EXP_SUCCEED:
            this->lexer.restore_state(lexer_state);
            return 1;
        default:
            return -1;
    }
}