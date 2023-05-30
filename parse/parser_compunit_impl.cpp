#include "parser.h"
#include "symbols.h"
#include "../utils.h"
#include <stdio.h>

int Parser::parse()
{
    while (true)
    {
        auto lexer_state = this->lexer.get_state();
        if (this->parse_next_decl().first) /* Next is a declaration */
        {
            auto lexer_state = this->lexer.get_state();
            if (this->lexer.next_lexeme().to_val() == END_OF_FILE)
                return 1;
            this->lexer.restore_state(lexer_state);
            continue;
        }
        this->lexer.restore_state(lexer_state);
        
        if (this->parse_next_funcdef())
        {
            auto lexer_state = this->lexer.get_state();
            if (this->lexer.next_lexeme().to_val() == END_OF_FILE)
                return 1;
            this->lexer.restore_state(lexer_state);
            continue;
        }
        fprintf(stderr, "Parser error: can't parse the input program at "
                "line %lu!\n", this->lexer.get_lineno());
        return 0;
    }
}