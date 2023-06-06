#include "utils.h"
#include "parse/symbols.h"

/**
 *  Read at most "num" bytes of code from file "filename", to char buffer "code".
 * 
 *  @param filename The file name to read from.
 *  @param code The char buffer.
 *  @param num The maximal number of bytes read.
 */
void read_file(const char* filename, char* code, std::size_t num)
{
    FILE* file = fopen(filename, "r");

    if (file == nullptr)
    {
        fprintf(stderr, "Error reading file!\n");
        exit(4);
    }

    for (std::size_t i = 0; i < num; i++)
    {
        code[i] = fgetc(file);
        if (code[i] == EOF)
        {
            code[i] = 0;
            break;
        }
    }

    fclose(file);
}

std::string array_type_to_str(const std::vector<std::size_t>& sizes)
{
    std::string base = "int";
    for (auto& size: sizes)
    {
        base += ("[" + std::to_string(size) + "]");
    }
    return base;
}

void print_stack(const std::stack<int>& stack)
{
    std::stack<int> stack_copy(stack);
    while (!stack_copy.empty())
    {
        printf("%d ", stack_copy.top());
        stack_copy.pop();
    }
    printf("\n");
}

void print_symbol_stack(const std::stack<void*>& stack)
{
    printf("\n");
    std::stack<void*> stack_copy(stack);
    while (!stack_copy.empty())
    {
        printf("%s\n", ((Symbol*)stack_copy.top())->to_str().c_str());
        stack_copy.pop();
    }
    printf("\n");   
}


