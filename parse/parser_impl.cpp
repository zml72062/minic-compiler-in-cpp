#include "parser.h"

Parser::Parser(const char* code): lexer(code)
{
    this->error = 0;
}

Parser::~Parser()
{

}

