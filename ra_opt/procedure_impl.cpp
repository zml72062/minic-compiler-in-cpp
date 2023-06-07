#include "basicblock.h"

Procedure::Procedure(const std::vector<IntermediateCode*>& _code)
{
    code = _code;
}

static std::vector<IntermediateCode*>
make_code_weakcopy(const std::vector<IntermediateCode*>& code, std::pair<std::size_t, std::size_t> range)
{
    std::vector<IntermediateCode*> code_copy;
    for (auto i = range.first; i < range.second; i++)
    {
        code_copy.push_back(code[i]);
    }
    return code_copy;
}

std::vector<Procedure*> make_procedures(const std::vector<IntermediateCode*>& code)
{
    std::vector<Procedure*> procs;
    auto length = code.size();
    std::size_t start = 0;
    for (auto& symtab_entry: symbol_table->get_entries())
    {
        /* Skip definition for SysY built-in functions. */
        if (symtab_entry->name == "getint" ||
            symtab_entry->name == "getch" ||
            symtab_entry->name == "getarray" ||
            symtab_entry->name == "putint" ||
            symtab_entry->name == "putch" ||
            symtab_entry->name == "putarray" ||
            symtab_entry->name == "starttime" ||
            symtab_entry->name == "stoptime")
        {
            continue;
        }
        if (symtab_entry->type.basic_type == BASIC_TYPE_FUNC)
        {
            while (start < length)
            {
                if (code[start]->instr == INSTR_GLOB 
                 && code[start]->loperand == symtab_entry->addr)
                    break;
                start++;
            }
            start++;
            std::size_t end = start;
            while (end < length)
            {
                if (code[end]->instr == INSTR_GLOB)
                    break;
                end++;
            }
            procs.push_back(new Procedure(make_code_weakcopy(code, {start, end})));
            start = end;
        }
    }
    return procs;
}

void Procedure::print_code()
{
    for (auto& code_line: code)
    {
        printf("%s\n", code_line->to_str().c_str());
    }
}