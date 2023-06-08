#include "lex.h"
#include <string.h>
#include <stdio.h>

Lexer::Lexer(const char* _code)
{
    this->cur_idx = 0;
    this->lineno = 1;
    this->error = 0;
    this->code = _code; /* No copy */
    this->length = strlen(_code);
}

Lexer::~Lexer()
{
    /* No delete */
}

Lexeme Lexer::next_lexeme()
{
    if (this->cur_idx == length) /* Reaches EOF */
        return Lexeme(RESERVED, END_OF_FILE);

    char c1 = this->code[this->cur_idx];

    if (c1 == '/')
    {
        if (this->cur_idx + 1 == length)
        {
            this->cur_idx++;
            return Lexeme(RESERVED, DIVIDE_OPERATOR);
        }
        char c2 = code[this->cur_idx + 1];
        if (c2 == '/') /* In-line comment starts. */
        {
            std::size_t id = this->cur_idx + 2;
            while (id != length && code[id] != '\n')
                id++;
            this->cur_idx = id;
            return this->next_lexeme();
        }
        else if (c2 == '*') /* Block comment starts. */
        {
            std::size_t id = this->cur_idx + 2;
            while (id != length)
            {
                if (code[id] == '*' &&
                    id + 1 != length &&
                    code[id + 1] == '/')
                {
                    this->cur_idx = id + 2;
                    return this->next_lexeme();
                }
                else if (code[id] == '\n')
                {
                    this->lineno++;
                }
                id++;
            }
            fprintf(stderr, "Lexical error: unclosed block comment at line %lu!\n",
                    this->lineno);
            this->cur_idx = id;
            this->error = 1;
            return Lexeme(ERROR);
        }
        else
        {
            this->cur_idx++;
            return Lexeme(RESERVED, DIVIDE_OPERATOR);
        }
    }
    else if (c1 == ' ' || c1 == '\t' || c1 == '\n' || c1 == '\r') /* Whitespaces. */
    {
        if (c1 == '\n')
            this->lineno++;
        std::size_t id = this->cur_idx + 1;
        while (id != length && 
               (code[id] == ' ' || code[id] == '\t' || code[id] == '\n' || code[id] == '\r'))
        {
            if (code[id] == '\n')
                this->lineno++;
            id++;
        }
        this->cur_idx = id;
        return this->next_lexeme();
    }
    else if (c1 == '0')
    {
        if (this->cur_idx + 1 == length)
        {
            this->cur_idx++;
            return Lexeme(NUMBERS, 0);
        }
        char c2 = code[this->cur_idx + 1];
        if (c2 == 'x' || c2 == 'X') /* Hexadecimal number. */
        {
            this->cur_idx += 2;
            std::size_t id = this->cur_idx;
            while (id != length &&
                   (code[id] >= '0' && code[id] <= '9' ||
                    code[id] >= 'a' && code[id] <= 'f' ||
                    code[id] >= 'A' && code[id] <= 'F'))
            {
                id++;
            }
            if (id == this->cur_idx)
            {
                fprintf(stderr, "Lexical error: invalid hexadecimal integer at "
                        "line %lu!\n", this->lineno);
                this->error = 1;
                return Lexeme(ERROR);
            }
            LexemeString slice(code, this->cur_idx, id - this->cur_idx);
            int value;
            sscanf(slice.c_str(), "%x", &value);

            this->cur_idx = id;
            return Lexeme(NUMBERS, value);
        }
        else if (c2 >= '0' && c2 <= '7') /* Octal number. */
        {
            this->cur_idx += 1;
            std::size_t id = this->cur_idx + 1;
            while (id != length && code[id] >= '0' && code[id] <= '7')
            {
                id++;
            }
            LexemeString slice(code, this->cur_idx, id - this->cur_idx);
            int value;
            sscanf(slice.c_str(), "%o", &value);

            this->cur_idx = id;
            return Lexeme(NUMBERS, value);
        }
        else 
        {
            this->cur_idx++;
            return Lexeme(NUMBERS, 0);
        }
    }
    else if (c1 >= '1' && c1 <= '9') /* Decimal number. */
    {
        std::size_t id = this->cur_idx + 1;
        while (id != length && code[id] >= '0' && code[id] <= '9')
            id++;
        LexemeString slice(code, this->cur_idx, id - this->cur_idx);
        int value;
        sscanf(slice.c_str(), "%d", &value);

        this->cur_idx = id;
        return Lexeme(NUMBERS, value);
    }
    else if (c1 >= 'a' && c1 <= 'z' ||
             c1 >= 'A' && c1 <= 'Z' ||
             c1 == '_') /* Keywords and identifiers. */
    {
        std::size_t id = this->cur_idx + 1;
        while (id != length &&
                (code[id] >= '0' && code[id] <= '9' ||
                 code[id] >= 'a' && code[id] <= 'z' ||
                 code[id] >= 'A' && code[id] <= 'Z' ||
                 code[id] == '_'))
        {
            id++;
        }
        LexemeString slice(code, this->cur_idx, id - this->cur_idx);
        this->cur_idx = id;
        if (!strcmp(slice.c_str(), "const"))
            return Lexeme(RESERVED, CONST_QUALIFIER);
        else if (!strcmp(slice.c_str(), "int"))
            return Lexeme(RESERVED, INT_TYPE);
        else if (!strcmp(slice.c_str(), "void"))
            return Lexeme(RESERVED, VOID_TYPE);
        else if (!strcmp(slice.c_str(), "if"))
            return Lexeme(RESERVED, IF_KEYWORD);
        else if (!strcmp(slice.c_str(), "else"))
            return Lexeme(RESERVED, ELSE_KEYWORD);
        else if (!strcmp(slice.c_str(), "while"))
            return Lexeme(RESERVED, WHILE_KEYWORD);
        else if (!strcmp(slice.c_str(), "break"))
            return Lexeme(RESERVED, BREAK_KEYWORD);
        else if (!strcmp(slice.c_str(), "continue"))
            return Lexeme(RESERVED, CONTINUE_KEYWORD);
        else if (!strcmp(slice.c_str(), "return"))
            return Lexeme(RESERVED, RETURN_KEYWORD);
        else
            return Lexeme(IDENTIFIERS, slice.c_str());
    }
    else 
    {
        this->cur_idx++;
        switch (c1)
        {
            case '+':
                return Lexeme(RESERVED, PLUS_OPERATOR);
            case '-':
                return Lexeme(RESERVED, MINUS_OPERATOR);
            case '*':
                return Lexeme(RESERVED, TIMES_OPERATOR);
            case '%':
                return Lexeme(RESERVED, MOD_OPERATOR);
            case '{':
                return Lexeme(RESERVED, LBRACE);
            case '}':
                return Lexeme(RESERVED, RBRACE);
            case '[':
                return Lexeme(RESERVED, LINDEX);
            case ']':
                return Lexeme(RESERVED, RINDEX);
            case '(':
                return Lexeme(RESERVED, LPAREN);
            case ')':
                return Lexeme(RESERVED, RPAREN);
            case ',':
                return Lexeme(RESERVED, COMMA);
            case ';':
                return Lexeme(RESERVED, SEMICOLON);
            case '&':
                if (this->cur_idx == length ||
                    code[this->cur_idx] != '&')
                {
                    fprintf(stderr, "Lexical error: invalid character & "
                    "at line %lu!\n", this->lineno);
                    this->error = 1;
                    return Lexeme(ERROR);
                }
                this->cur_idx++;
                return Lexeme(RESERVED, AND_OPERATOR);
            case '|':
                if (this->cur_idx == length ||
                    code[this->cur_idx] != '|')
                {
                    fprintf(stderr, "Lexical error: invalid character | "
                    "at line %lu!\n", this->lineno);
                    this->error = 1;
                    return Lexeme(ERROR);
                }
                this->cur_idx++;
                return Lexeme(RESERVED, OR_OPERATOR);
            case '!':
                if (this->cur_idx == length ||
                    code[this->cur_idx] != '=')
                    return Lexeme(RESERVED, NOT_OPERATOR);
                this->cur_idx++;
                return Lexeme(RESERVED, NEQ_OPERATOR);
            case '=':
                if (this->cur_idx == length ||
                    code[this->cur_idx] != '=')
                    return Lexeme(RESERVED, ASSIGN);
                this->cur_idx++;
                return Lexeme(RESERVED, EQ_OPERATOR);
            case '<':
                if (this->cur_idx == length ||
                    code[this->cur_idx] != '=')
                    return Lexeme(RESERVED, L_OPERATOR);
                this->cur_idx++;
                return Lexeme(RESERVED, LEQ_OPERATOR);
            case '>':
                if (this->cur_idx == length ||
                    code[this->cur_idx] != '=')
                    return Lexeme(RESERVED, G_OPERATOR);
                this->cur_idx++;
                return Lexeme(RESERVED, GEQ_OPERATOR);
            default:
                fprintf(stderr, "Lexical error: invalid character %c "
                    "at line %lu!\n", c1, this->lineno);
                this->error = 1;
                return Lexeme(ERROR);
        }
    }
}


std::size_t Lexer::get_lineno()
{
    return this->lineno;
}

void Lexer::restore_state(const std::vector<std::size_t>& state)
{
    this->cur_idx = state[0];
    this->lineno = state[1];
    this->error = (int) state[2];
}

std::vector<std::size_t> Lexer::get_state()
{
    std::vector<std::size_t> state;
    state.push_back(this->cur_idx);
    state.push_back(this->lineno);
    state.push_back((std::size_t)error);
    return state;
}

