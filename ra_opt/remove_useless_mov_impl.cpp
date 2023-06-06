#include "basicblock.h"

void remove_useless_mov(std::vector<IntermediateCode*>& code)
{
    auto length = code.size();
    auto begin = code.begin();
    for (std::size_t i = 0; i < length; i++)
    {
        if (code[i]->instr == INSTR_RRMOV && code[i]->dest == code[i]->loperand
         && length > 1 && (i == length - 1 || code[i + 1]->labels.size() == 0))
        {
            std::vector<std::size_t> labels(code[i]->labels);
            delete code[i];
            code.erase(begin + i);
            if (i < length - 1)
                (*(begin + i))->labels = labels;
            length--;
            i--;
        }
    }
}