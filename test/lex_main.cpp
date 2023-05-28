#include "../lex/lex.h"
#include "../utils.h"
#include <stdio.h>

#define MAX_CODE_LENGTH 16384

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    char code[MAX_CODE_LENGTH + 1] = {0};
    read_file(argv[1], code, MAX_CODE_LENGTH);

    Lexer lexer(code);
    while (true)
    {
        auto lexeme = lexer.next_lexeme();
        if (lexeme.lex_type == ERROR)
            exit(1);
        printf("%s\n", lexeme.to_str().c_str());

        if (lexeme.lex_type == RESERVED &&
            lexeme.lex_value.value == END_OF_FILE)
            break;
    }
    return 0;
}