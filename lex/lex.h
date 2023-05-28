#ifndef LEX_H
#define LEX_H

#include <cstddef>
#include <string>
#include <vector>

/* Qualifiers. */
#define CONST_QUALIFIER 0
/* Types. */
#define INT_TYPE 1
#define VOID_TYPE 2
/* Keywords. */
#define IF_KEYWORD 3
#define ELSE_KEYWORD 4
#define WHILE_KEYWORD 5
#define BREAK_KEYWORD 6
#define CONTINUE_KEYWORD 7
#define RETURN_KEYWORD 8
/* Operators. */
#define PLUS_OPERATOR 9
#define MINUS_OPERATOR 10
#define NOT_OPERATOR 11
#define TIMES_OPERATOR 12
#define DIVIDE_OPERATOR 13
#define MOD_OPERATOR 14
#define AND_OPERATOR 15
#define OR_OPERATOR 16
#define EQ_OPERATOR 17
#define NEQ_OPERATOR 18
#define G_OPERATOR 19
#define GEQ_OPERATOR 20
#define L_OPERATOR 21
#define LEQ_OPERATOR 22
/* Other symbols. */
#define LBRACE 23
#define RBRACE 24
#define LPAREN 25
#define RPAREN 26
#define LINDEX 27
#define RINDEX 28
#define SEMICOLON 29
#define COMMA 30
#define ASSIGN 31
/* EOF. */
#define END_OF_FILE 32

/* Lexeme types. */
#define ERROR -1
#define RESERVED 0
#define NUMBERS 1
#define IDENTIFIERS 2

#define NUMBERS_VAL 10000
#define IDENTIFIERS_VAL 20000

using LexemeString = std::string;

struct LexemeValue {
    /* Currently, we only introduce a value for numbers and reserved */
    int value;

    void set(int _value);
};

struct Lexeme {
    /*  0 for reserved,
        1 for numbers,
        2 for identifiers */
    int lex_type; 
    LexemeString lex_name;
    LexemeValue lex_value;

    Lexeme(int _type);
    Lexeme(int _type, int _value);
    Lexeme(int _type, const char* _name);
    std::string to_str();
    int to_val();
    ~Lexeme();
};

class Lexer
{
private:
    std::size_t cur_idx;
    std::size_t lineno;
    std::size_t length;
    int error;
    const char* code;
public:
    std::size_t get_lineno();
    Lexer(const char* _code);
    ~Lexer();
    Lexeme next_lexeme();
    void restore_state(const std::vector<std::size_t>& state);
    std::vector<std::size_t> get_state();
};

#endif