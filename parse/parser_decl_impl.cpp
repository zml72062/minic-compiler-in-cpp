#include "parser.h"
#include <stdio.h>

/**
 *  The first component records whether the parsing is successful;
 *  1 for true and 0 for false.
 *  The second component records the number of symbol table entries
 *  appended to the symbol table.
 */
std::pair<int, int> Parser::parse_next_decl()
{
    auto parse_const = this->parse_next_const_decl();
    if (parse_const.first)
    {
        auto lexeme_val = this->lexer.next_lexeme().to_val();
        if (lexeme_val == SEMICOLON)
            return std::pair<int, int>(1, parse_const.second);
        for (auto i = 0; i < parse_const.second; i++)
            symbol_table->delete_entry();
        fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                this->lexer.get_lineno());
        this->error = 1;
        return std::pair<int, int>(0, 0);
    }
    auto parse_var = this->parse_next_var_decl();
    if (parse_var.first)
    {
        auto lexeme_val = this->lexer.next_lexeme().to_val();
        if (lexeme_val == SEMICOLON)
            return std::pair<int, int>(1, parse_var.second);
        for (auto i = 0; i < parse_var.second; i++)
            symbol_table->delete_entry();
        fprintf(stderr, "Syntactical error: expected ';' at line %lu!\n",
                this->lexer.get_lineno());
        this->error = 1;
        return std::pair<int, int>(0, 0);
    }
    return std::pair<int, int>(0, 0);
}