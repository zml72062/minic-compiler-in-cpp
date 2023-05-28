#include "lex.h"

Lexeme::Lexeme(int _type): lex_name(), lex_value()
{
    this->lex_type = _type;
}

Lexeme::Lexeme(int _type, int _value): lex_name(), lex_value()
{
    this->lex_type = _type;
    this->lex_value.set(_value);
}

Lexeme::Lexeme(int _type, const char* _name): lex_name(_name), lex_value()
{
    this->lex_type = _type;
}

Lexeme::~Lexeme()
{
    
}

int Lexeme::to_val()
{
    if (this->lex_type == RESERVED)
        return this->lex_value.value;
    else 
        return this->lex_type * 10000;
}

std::string Lexeme::to_str()
{
    const char* reserved[] = {
        "CONST_QUALIFIER",
        "INT_TYPE",
        "VOID_TYPE",
        "IF_KEYWORD",
        "ELSE_KEYWORD",
        "WHILE_KEYWORD",
        "BREAK_KEYWORD",
        "CONTINUE_KEYWORD",
        "RETURN_KEYWORD",
        "PLUS_OPERATOR",
        "MINUS_OPERATOR",
        "NOT_OPERATOR",
        "TIMES_OPERATOR",
        "DIVIDE_OPERATOR",
        "MOD_OPERATOR",
        "AND_OPERATOR",
        "OR_OPERATOR",
        "EQ_OPERATOR",
        "NEQ_OPERATOR",
        "G_OPERATOR",
        "GEQ_OPERATOR",
        "L_OPERATOR",
        "LEQ_OPERATOR",
        "LBRACE",
        "RBRACE",
        "LPAREN",
        "RPAREN",
        "LINDEX",
        "RINDEX",
        "SEMICOLON",
        "COMMA",
        "ASSIGN",
        "END_OF_FILE"
    };
    if (this->lex_type == RESERVED)
        return std::string(reserved[this->lex_value.value]) + "()";
    else if (this->lex_type == NUMBERS)
        return "NUMBER(" + std::to_string(this->lex_value.value) + ")";
    else if (this->lex_type == IDENTIFIERS)
        return "IDENTIFIER(" + this->lex_name + ")";
    else
        return std::string("ERROR");
}
