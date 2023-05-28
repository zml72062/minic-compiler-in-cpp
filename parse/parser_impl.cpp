#include "parser.h"

Parser::Parser(const char* code): states(), symbols(), lexer(code)
{
    states.push(PROGRAM_START);
    this->error = 0;
}

Parser::~Parser()
{

}

int Parser::cur_state()
{
    return states.top();
}

