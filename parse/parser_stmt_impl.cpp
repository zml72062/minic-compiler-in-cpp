#include "parser.h"

void Parser::parse_next()
{
    int state = this->cur_state();
    auto lexer_state = this->lexer.get_state();
    auto next_lexeme = this->lexer.next_lexeme();
    int next_val = next_lexeme.to_val();

    switch (state)
    {
        case PROGRAM_START:
            if (next_val == CONST_QUALIFIER)
            {
                states.push(GET_CONST_WAIT_FOR_INT);
                return;
            }
            if (next_val == INT_TYPE)
            {
                states.push(GET_INT_WAIT_FOR_IDENT);
                return;
            }
            if (next_val == VOID_TYPE)
            {
                states.push(GET_VOID_WAIT_FOR_IDENT);
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_CONST_WAIT_FOR_INT:
            if (next_val == INT_TYPE)
            {
                states.push(GET_CONST_INT_WAIT_FOR_IDENT);
                return;
            }
            fprintf(stderr, "Syntactical error: 'const' qualifier "
                    "must be followed by 'int' at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_INT_WAIT_FOR_IDENT:
            /* Only push lexeme to stack when lexeme is non-trivial,
               i.e. identifier or number. */
            if (next_val == IDENTIFIERS_VAL)
            {
                states.push(GET_INT_DECLARE);
                symbols.push(new LexemePacker(next_lexeme));
                return;
            }
            fprintf(stderr, "Syntactical error: 'int' type must "
                    "be followed by an identifier at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_VOID_WAIT_FOR_IDENT:
            if (next_val == IDENTIFIERS_VAL)
            {
                states.push(GET_VOID_DECLARE);
                symbols.push(new LexemePacker(next_lexeme));
                return;
            }
            fprintf(stderr, "Syntactical error: 'void' type must "
                    "be followed by an identifier at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_CONST_INT_WAIT_FOR_IDENT:
            if (next_val == IDENTIFIERS_VAL)
            {
                states.push(GET_CONST_INT_DECLARE);
                symbols.push(new LexemePacker(next_lexeme));
                return;
            }
            fprintf(stderr, "Syntactical error: 'const int' type must "
                    "be followed by an identifier at line %lu!\n",
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_INT_DECLARE:
            if (next_val == SEMICOLON ||
                next_val == COMMA ||
                next_val == ASSIGN ||
                next_val == LINDEX)
            {
                /**
                 * Add the symbol to symbol table.
                 */
                LexemePacker* top_symbol = (LexemePacker*)(symbols.top());
                
                /* Detect re-definition. */
                std::string name(top_symbol->lexeme.lex_name);
                if (symbol_table->get_entry_if_contains(name.c_str()) != nullptr)
                {
                    fprintf(stderr, "Semantic error: name '%s' "
                            "redefined at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return;
                }

                symbol_table->add_entry(
                    SymbolTableEntry(name, BASIC_TYPE_INT)
                );
                states.pop();
                states.push(GET_VAR_DEF_WITHOUT_INIT);
                this->lexer.restore_state(lexer_state);
                return;
            }
            if (next_val == LPAREN)
            {
                states.push(GET_INT_FUNC_DECLARE);
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_VOID_DECLARE:
            if (next_val == LPAREN)
            {
                states.push(GET_VOID_FUNC_DECLARE);
                return;
            }
            if (next_val == SEMICOLON ||
                next_val == COMMA ||
                next_val == ASSIGN ||
                next_val == LINDEX)
            {
                fprintf(stderr, "Syntactical error: cannot define variables "
                        "with 'void' type at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_CONST_INT_DECLARE:
            if (next_val == ASSIGN ||
                next_val == LINDEX)
            {
                /**
                 * Add the symbol to symbol table.
                 */
                LexemePacker* top_symbol = (LexemePacker*)(symbols.top());
                
                /* Detect re-definition. */
                std::string name(top_symbol->lexeme.lex_name);
                if (symbol_table->get_entry_if_contains(name.c_str()) != nullptr)
                {
                    fprintf(stderr, "Semantic error: name '%s' "
                            "redefined at line %lu!\n", name.c_str(),
                            this->lexer.get_lineno());
                    this->error = 1;
                    return;
                }

                symbol_table->add_entry(
                    SymbolTableEntry(name, BASIC_TYPE_CONST_INT)
                );
                states.pop();
                states.push(GET_CONST_DEF_WITHOUT_INIT);
                this->lexer.restore_state(lexer_state);
                return;
            }
            if (next_val == SEMICOLON ||
                next_val == COMMA)
            {
                fprintf(stderr, "Syntactical error: constant with 'const int' "
                        "type requires initialization at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            if (next_val == LPAREN)
            {
                fprintf(stderr, "Syntactical error: cannot define functions "
                        "with return type 'const int' at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_INT_FUNC_DECLARE:
            if (next_val == INT_TYPE)
            {
                states.push(GET_INT_ARG_TYPE);
                return;
            }
            if (next_val == RPAREN)
            {
                states.push(GET_NON_ARG_WITH_RET_INT);
                return;
            }
            if (next_val == CONST_QUALIFIER)
            {
                fprintf(stderr, "Syntactical error: 'const int' cannot "
                        "be an argument type at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            if (next_val == VOID_TYPE)
            {
                fprintf(stderr, "Syntactical error: 'void' cannot "
                        "be an argument type at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_VOID_FUNC_DECLARE:
            if (next_val == INT_TYPE)
            {
                states.push(GET_INT_ARG_TYPE);
                return;
            }
            if (next_val == RPAREN)
            {
                states.push(GET_NON_ARG_WITH_RET_VOID);
                return;
            }
            if (next_val == CONST_QUALIFIER)
            {
                fprintf(stderr, "Syntactical error: 'const int' cannot "
                        "be an argument type at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            if (next_val == VOID_TYPE)
            {
                fprintf(stderr, "Syntactical error: 'void' cannot "
                        "be an argument type at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case GET_CONST_DEF_WITHOUT_INIT:
            if (next_val == ASSIGN)
            {
                states.push(WAIT_FOR_CONST_INIT);
                return;
            }
            if (next_val == LINDEX)
            {
                states.push(WAIT_FOR_CONST_ARRAY_LEN);
                return;
            }
            if (next_val == COMMA || 
                next_val == SEMICOLON)
            {
                fprintf(stderr, "Syntactical error: constants with 'const int' "
                        "type must be initialized at line %lu!\n",
                        this->lexer.get_lineno());
                this->error = 1;
                return;
            }
            fprintf(stderr, "Syntactical error: unexpected token %s "
                    "at line %lu!\n", next_lexeme.to_str().c_str(),
                    this->lexer.get_lineno());
            this->error = 1;
            return;
        case WAIT_FOR_VAR_INIT:
            /* Before calling "parse_next_exp()", must restore lexer state. */
            return;

    }
}
