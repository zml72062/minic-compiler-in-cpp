#include "parser.h"
#include "symbols.h"
#include "../utils.h"
#include <stdio.h>

Symbol* Parser::parse_next_block()
{
    std::stack<int> block_states;
    std::stack<void*> block_symbols;
    block_states.push(BLOCK_START_BLOCK);
    auto lexer_state = this->lexer.get_state();

    while (true)
    {
        int can_parse = this->parse_block_next_step(block_states, block_symbols);
        if (can_parse == -1)
        {
            this->lexer.restore_state(lexer_state);
            while(!block_symbols.empty())
            {
                clear((Symbol*)block_symbols.top());
                block_symbols.pop();
            }
            return nullptr;
        }
        if (can_parse == 1)
            return (Symbol*)block_symbols.top();
    }
}

int Parser::parse_block_next_step(std::stack<int>& _states, 
                                  std::stack<void*>& _symbols)
{
    int state = _states.top();
    auto lexer_state = this->lexer.get_state();
    auto next_lexeme = this->lexer.next_lexeme();
    int next_val = next_lexeme.to_val();

    switch (state)
    {
        case BLOCK_START_BLOCK:
            if (next_val == LBRACE)
            {
                _states.push(BLOCK_ENABLE_NEW_BLOCK);

                /* Create a new scope. */
                symbol_table = symbol_table->create_subscope();
                _symbols.push(new Block());
                return 0;
            }
            return -1;
        case BLOCK_ENABLE_NEW_BLOCK:
            if (next_val == LBRACE)
            {
                _states.push(BLOCK_ENABLE_NEW_BLOCK);

                /* Create a new scope. */
                symbol_table = symbol_table->create_subscope();
                _symbols.push(new Block());
                return 0;
            }
            if (next_val == RBRACE)
            {
                _states.push(BLOCK_GET_EMPTY_BLOCK);

                symbol_table = symbol_table->parent_scope();
                return 0;
            }
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_EMPTY_STMT);
                return 0;
            }
            if (next_val == WHILE_KEYWORD)
            {
                _states.push(BLOCK_GET_WHILE);
                return 0;
            }
            if (next_val == IF_KEYWORD)
            {
                _states.push(BLOCK_GET_IF);
                return 0;
            }
            if (next_val == RETURN_KEYWORD)
            {
                _states.push(BLOCK_GET_RETURN);
                return 0;
            }
            if (next_val == CONTINUE_KEYWORD)
            {
                _states.push(BLOCK_GET_CONTINUE);
                return 0;
            }
            if (next_val == BREAK_KEYWORD)
            {
                _states.push(BLOCK_GET_BREAK);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto next_decl = this->parse_next_decl();
                if (next_decl.first)
                {
                    _states.push(BLOCK_GET_DECL);

                    Block* block = (Block*)(_symbols.top());
                    auto entry_number = symbol_table->get_entries().size();
                    auto last_appended_entry_number = next_decl.second;
                    for (auto i = entry_number - last_appended_entry_number;
                         i < entry_number; i++)
                    {
                        auto entry = symbol_table->get_entries()[i];
                        /* Only add declaration for variables. */
                        if (entry->type.basic_type == BASIC_TYPE_INT)
                        {
                            block->add_statement(new LocalVarDeclaration(entry));
                        }
                    }

                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected a statement at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                if (exp->is_lval())
                {
                    auto lexer_state_after_exp = this->lexer.get_state();
                    auto next_val = this->lexer.next_lexeme().to_val();

                    if (next_val == ASSIGN)
                    {
                        this->lexer.restore_state(lexer_state_after_exp);
                        _states.push(BLOCK_GET_LVAL);
                        _symbols.push(exp);
                        return 0;
                    }
                    this->lexer.restore_state(lexer_state_after_exp);
                }
                _states.push(BLOCK_GET_EXP);
                _symbols.push(exp);
                return 0;
            }
        case BLOCK_GET_BREAK:
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_BREAK_STMT);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_CONTINUE:
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_CONTINUE_STMT);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_RETURN:
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_EMPTY_RETURN_STMT);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                _states.push(BLOCK_GET_RETURN_VALUE);
                _symbols.push(exp);
                return 0;
            }
        case BLOCK_GET_RETURN_VALUE:
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_NON_EMPTY_RETURN_STMT);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_IF:
            if (next_val == LPAREN)
            {
                _states.push(BLOCK_GET_IF_LPAREN);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected '(' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_WHILE:
            if (next_val == LPAREN)
            {
                _states.push(BLOCK_GET_WHILE_LPAREN);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected '(' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_IF_LPAREN:
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                _states.push(BLOCK_GET_IF_EXP_WAIT_FOR_RPAREN);
                _symbols.push(exp);
                return 0;
            }
        case BLOCK_GET_WHILE_LPAREN:
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                _states.push(BLOCK_GET_WHILE_EXP_WAIT_FOR_RPAREN);
                _symbols.push(exp);
                return 0;
            }
        case BLOCK_GET_IF_EXP_WAIT_FOR_RPAREN:
            if (next_val == RPAREN)
            {
                _states.push(BLOCK_GET_IF_EXP);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ')' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_WHILE_EXP_WAIT_FOR_RPAREN:
            if (next_val == RPAREN)
            {
                _states.push(BLOCK_GET_WHILE_EXP);
                return 0;
            }
            fprintf(stderr, "Syntactical error: expected ')' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_IF_EXP:
        case BLOCK_GET_WHILE_EXP:
        case BLOCK_GET_IF_STMT_AND_ELSE:
            if (next_val == LBRACE)
            {
                _states.push(BLOCK_ENABLE_NEW_BLOCK);

                /* Create a new scope. */
                symbol_table = symbol_table->create_subscope();
                _symbols.push(new Block());
                return 0;
            }
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_EMPTY_STMT);
                return 0;
            }
            if (next_val == WHILE_KEYWORD)
            {
                _states.push(BLOCK_GET_WHILE);
                return 0;
            }
            if (next_val == IF_KEYWORD)
            {
                _states.push(BLOCK_GET_IF);
                return 0;
            }
            if (next_val == RETURN_KEYWORD)
            {
                _states.push(BLOCK_GET_RETURN);
                return 0;
            }
            if (next_val == CONTINUE_KEYWORD)
            {
                _states.push(BLOCK_GET_CONTINUE);
                return 0;
            }
            if (next_val == BREAK_KEYWORD)
            {
                _states.push(BLOCK_GET_BREAK);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected a statement at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                if (exp->is_lval())
                {
                    auto lexer_state_after_exp = this->lexer.get_state();
                    auto next_val = this->lexer.next_lexeme().to_val();

                    if (next_val == ASSIGN)
                    {
                        this->lexer.restore_state(lexer_state_after_exp);
                        _states.push(BLOCK_GET_LVAL);
                        _symbols.push(exp);
                        return 0;
                    }
                    this->lexer.restore_state(lexer_state_after_exp);
                }
                _states.push(BLOCK_GET_EXP);
                _symbols.push(exp);
                return 0;
            }
        case BLOCK_GET_EMPTY_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                _symbols.push(new EmptyStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    _symbols.push(new EmptyStatement());
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                _symbols.push(new EmptyStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case BLOCK_GET_BREAK_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                _symbols.push(new BreakStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    _symbols.push(new BreakStatement());
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                _symbols.push(new BreakStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case BLOCK_GET_CONTINUE_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                _symbols.push(new ContinueStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    _symbols.push(new ContinueStatement());
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                _symbols.push(new ContinueStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case BLOCK_GET_EMPTY_RETURN_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                _symbols.push(new ReturnNoneStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    _symbols.push(new ReturnNoneStatement());
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                _symbols.push(new ReturnNoneStatement());
                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case BLOCK_GET_IF_ELSE_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                
                Symbol* else_stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* if_stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new IfElseStatement(expr, if_stmt, else_stmt));

                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    
                    Symbol* else_stmt = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    Symbol* if_stmt = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    Symbol* expr = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    _symbols.push(new IfElseStatement(expr, if_stmt, else_stmt));

                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* else_stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* if_stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new IfElseStatement(expr, if_stmt, else_stmt));

                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case BLOCK_GET_IF_STMT:
            if (next_val == ELSE_KEYWORD)
            {
                _states.push(BLOCK_GET_IF_STMT_AND_ELSE);
                return 0;
            }
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }

                Symbol* stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new IfStatement(expr, stmt));
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    Symbol* stmt = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    Symbol* expr = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    _symbols.push(new IfStatement(expr, stmt));
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    this->lexer.restore_state(lexer_state);
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new IfStatement(expr, stmt));
                this->lexer.restore_state(lexer_state);
                return 0;
            }
        case BLOCK_GET_WHILE_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }

                Symbol* stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new WhileStatement(expr, stmt));
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    Symbol* stmt = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    Symbol* expr = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    _symbols.push(new WhileStatement(expr, stmt));
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new WhileStatement(expr, stmt));
                this->lexer.restore_state(lexer_state);
                return 0;
            }            
        case BLOCK_GET_NON_EMPTY_RETURN_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }

                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new ReturnValueStatement(expr));
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    Symbol* expr = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    _symbols.push(new ReturnValueStatement(expr));
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new ReturnValueStatement(expr));
                this->lexer.restore_state(lexer_state);
                return 0;
            }   
        case BLOCK_GET_LVAL:
            if (next_val == ASSIGN)
            {
                _states.push(BLOCK_GET_LVAL_ASSIGN);
                return 0;
            }
            return -1;
        case BLOCK_GET_LVAL_ASSIGN:
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                {
                    fprintf(stderr, "Syntactical error: expected an expression at line %lu!\n",
                            this->lexer.get_lineno());
                    this->error = 1;
                    return -1;
                }
                _states.push(BLOCK_GET_ASSIGN_STMT_WITHOUT_SEMICOLON);
                _symbols.push(exp);
                return 0;
            }  
        case BLOCK_GET_ASSIGN_STMT_WITHOUT_SEMICOLON:
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_ASSIGN_STMT);
                return 0;
            } 
            fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_ASSIGN_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }

                Symbol* rval = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* lval = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new AssignStatement(lval, rval));
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    Symbol* rval = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    Symbol* lval = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    _symbols.push(new AssignStatement(lval, rval));
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* rval = (Symbol*)(_symbols.top());
                _symbols.pop();
                Symbol* lval = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new AssignStatement(lval, rval));
                this->lexer.restore_state(lexer_state);
                return 0;
            }    
        case BLOCK_GET_EXP:
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_EXPR_STMT);
                return 0;
            } 
            fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return -1;
        case BLOCK_GET_EXPR_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }

                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new ExpressionStatement(expr));
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    Symbol* expr = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    _symbols.push(new ExpressionStatement(expr));
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* expr = (Symbol*)(_symbols.top());
                _symbols.pop();
                _symbols.push(new ExpressionStatement(expr));
                this->lexer.restore_state(lexer_state);
                return 0;
            }  
        case BLOCK_GET_BLOCK_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == ELSE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_NEW_STMT);
                            break;
                        case BLOCK_GET_WHILE_EXP:
                            _states.push(BLOCK_GET_WHILE_STMT);
                            break;
                        case BLOCK_GET_IF_EXP:
                            _states.push(BLOCK_GET_IF_STMT);
                            break;
                        case BLOCK_GET_IF_STMT_AND_ELSE:
                            _states.push(BLOCK_GET_IF_ELSE_STMT);
                            break;
                        default:
                            return -1;
                    }
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_NEW_STMT);
                        break;
                    case BLOCK_GET_WHILE_EXP:
                        _states.push(BLOCK_GET_WHILE_STMT);
                        break;
                    case BLOCK_GET_IF_EXP:
                        _states.push(BLOCK_GET_IF_STMT);
                        break;
                    case BLOCK_GET_IF_STMT_AND_ELSE:
                        _states.push(BLOCK_GET_IF_ELSE_STMT);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                this->lexer.restore_state(lexer_state);
                return 0;
            }  
        case BLOCK_GET_NEW_STMT:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                        _states.push(BLOCK_GET_FIRST_BLOCK_ITEM);
                        break;
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_ONE_MORE_BLOCK_ITEM);
                        break;
                    default:
                        return -1;
                }

                Symbol* stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Block* block = (Block*)(_symbols.top());
                block->add_statement(stmt);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                            _states.push(BLOCK_GET_FIRST_BLOCK_ITEM);
                            break;
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_ONE_MORE_BLOCK_ITEM);
                            break;
                        default:
                            return -1;
                    }

                    Symbol* stmt = (Symbol*)(_symbols.top());
                    _symbols.pop();
                    Block* block = (Block*)(_symbols.top());
                    block->add_statement(stmt);
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                        _states.push(BLOCK_GET_FIRST_BLOCK_ITEM);
                        break;
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_ONE_MORE_BLOCK_ITEM);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                Symbol* stmt = (Symbol*)(_symbols.top());
                _symbols.pop();
                Block* block = (Block*)(_symbols.top());
                block->add_statement(stmt);
                this->lexer.restore_state(lexer_state);
                return 0;
            }  
        case BLOCK_GET_DECL:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                        _states.push(BLOCK_GET_FIRST_BLOCK_ITEM);
                        break;
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_ONE_MORE_BLOCK_ITEM);
                        break;
                    default:
                        return -1;
                }
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl(); 
                if (decl_result.first)
                {
                    _states.pop();
                    switch (_states.top())
                    {
                        case BLOCK_ENABLE_NEW_BLOCK:
                            _states.push(BLOCK_GET_FIRST_BLOCK_ITEM);
                            break;
                        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                            _states.push(BLOCK_GET_ONE_MORE_BLOCK_ITEM);
                            break;
                        default:
                            return -1;
                    }
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }  
            } 
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                switch (_states.top())
                {
                    case BLOCK_ENABLE_NEW_BLOCK:
                        _states.push(BLOCK_GET_FIRST_BLOCK_ITEM);
                        break;
                    case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
                        _states.push(BLOCK_GET_ONE_MORE_BLOCK_ITEM);
                        break;
                    default:
                        return -1;
                }
                clear(exp);
                this->lexer.restore_state(lexer_state);
                return 0;
            }  
        case BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT:
            if (next_val == LBRACE)
            {
                _states.push(BLOCK_ENABLE_NEW_BLOCK);

                /* Create a new scope. */
                symbol_table = symbol_table->create_subscope();
                _symbols.push(new Block());
                return 0;
            }
            if (next_val == RBRACE)
            {
                _states.push(BLOCK_GET_NON_EMPTY_BLOCK);

                symbol_table = symbol_table->parent_scope();
                return 0;
            }
            if (next_val == SEMICOLON)
            {
                _states.push(BLOCK_GET_EMPTY_STMT);
                return 0;
            }
            if (next_val == WHILE_KEYWORD)
            {
                _states.push(BLOCK_GET_WHILE);
                return 0;
            }
            if (next_val == IF_KEYWORD)
            {
                _states.push(BLOCK_GET_IF);
                return 0;
            }
            if (next_val == RETURN_KEYWORD)
            {
                _states.push(BLOCK_GET_RETURN);
                return 0;
            }
            if (next_val == CONTINUE_KEYWORD)
            {
                _states.push(BLOCK_GET_CONTINUE);
                return 0;
            }
            if (next_val == BREAK_KEYWORD)
            {
                _states.push(BLOCK_GET_BREAK);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto next_decl = this->parse_next_decl();
                if (next_decl.first)
                {
                    _states.push(BLOCK_GET_DECL);

                    Block* block = (Block*)(_symbols.top());
                    auto entry_number = symbol_table->get_entries().size();
                    auto last_appended_entry_number = next_decl.second;
                    for (auto i = entry_number - last_appended_entry_number;
                         i < entry_number; i++)
                    {
                        auto entry = symbol_table->get_entries()[i];
                        /* Only add declaration for variables. */
                        if (entry->type.basic_type == BASIC_TYPE_INT)
                        {
                            block->add_statement(new LocalVarDeclaration(entry));
                        }
                    }

                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                if (exp->is_lval())
                {
                    auto lexer_state_after_exp = this->lexer.get_state();
                    auto next_val = this->lexer.next_lexeme().to_val();

                    if (next_val == ASSIGN)
                    {
                        this->lexer.restore_state(lexer_state_after_exp);
                        _states.push(BLOCK_GET_LVAL);
                        _symbols.push(exp);
                        return 0;
                    }
                    this->lexer.restore_state(lexer_state_after_exp);
                }
                _states.push(BLOCK_GET_EXP);
                _symbols.push(exp);
                return 0;
            }
        case BLOCK_GET_NON_EMPTY_BLOCK:
            _states.pop();
        case BLOCK_GET_EMPTY_BLOCK:
            _states.pop();
            _states.pop();
            if (_states.top() == BLOCK_START_BLOCK)
                _states.push(BLOCK_SUCCEED);
            else
                _states.push(BLOCK_GET_BLOCK_STMT);
            this->lexer.restore_state(lexer_state);
            return 0;
        case BLOCK_GET_ONE_MORE_BLOCK_ITEM:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.pop();
                _states.push(BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    _states.pop();
                    _states.push(BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT);
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.pop();
                _states.push(BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT);
                clear(exp);
                this->lexer.restore_state(lexer_state);
                return 0;
            }  
        case BLOCK_GET_FIRST_BLOCK_ITEM:
            if (next_val == LBRACE ||
                next_val == RBRACE ||
                next_val == SEMICOLON ||
                next_val == WHILE_KEYWORD ||
                next_val == IF_KEYWORD ||
                next_val == BREAK_KEYWORD ||
                next_val == CONTINUE_KEYWORD ||
                next_val == RETURN_KEYWORD)
            {
                _states.pop();
                _states.push(BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT);
                this->lexer.restore_state(lexer_state);
                return 0;
            }
            this->lexer.restore_state(lexer_state);
            {
                auto decl_result = this->parse_next_decl();
                if (decl_result.first)
                {
                    _states.pop();
                    _states.push(BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT);
                    this->lexer.restore_state(lexer_state);
                    for (auto i = 0; i < decl_result.second; i++)
                        symbol_table->delete_entry();
                    return 0;
                }
            }
            this->lexer.restore_state(lexer_state);
            {
                Symbol* exp = this->parse_next_exp();
                if (exp == nullptr)
                    return -1;
                _states.pop();
                _states.push(BLOCK_WAIT_FOR_OPTIONALLY_MORE_STMT);
                clear(exp);
                this->lexer.restore_state(lexer_state);
                return 0;
            }  
        case BLOCK_SUCCEED:
            this->lexer.restore_state(lexer_state);
            return 1;
    }
    return -1;
}
