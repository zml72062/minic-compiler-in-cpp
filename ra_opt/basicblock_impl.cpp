#include "basicblock.h"
#include <utility>
#include <map>
#include <algorithm>

/**
 *  Return a vector of labels, each of which is a head instruction.
 */
static std::vector<std::size_t>
find_head_instr(const std::vector<IntermediateCode*>& code)
{
    std::vector<std::size_t> head_instr;
    std::size_t length = code.size();
    for (std::size_t i = 0; i < length; i++)
    {
        if (code[i]->instr != INSTR_GLOB)
        {
            head_instr.push_back(i);
            break;
        }
    }
    head_instr.push_back(length);

    std::map<std::size_t, std::size_t> index_dict;
    /* 'index_dict' maps label number to line number. */
    for (std::size_t i = 0; i < length; i++)
    {
        if (code[i]->labels.size() > 0)
        {
            index_dict[code[i]->labels[0]] = i;
        }
    }
    for (std::size_t i = 0; i < length; i++)
    {
        if (code[i]->instr == INSTR_JMP ||
            code[i]->instr == INSTR_JE ||
            code[i]->instr == INSTR_JNE)
        {
            head_instr.push_back(index_dict.at(code[i]->roperand)); /* Find jump target. */
            head_instr.push_back(i + 1);
        }
    }
    std::sort(head_instr.begin(), head_instr.end());
    auto new_end = std::unique(head_instr.begin(), head_instr.end());
    return std::vector<std::size_t>(head_instr.begin(), new_end);
}

static std::vector<std::pair<std::size_t, std::size_t>>
find_basic_blocks(const std::vector<IntermediateCode*>& code)
{
    auto head_instr = find_head_instr(code);
    std::vector<std::pair<std::size_t, std::size_t>> basic_blocks;
    auto length = head_instr.size();
    for (std::size_t i = 0; i < length; i++)
    {
        if (i == length - 1) /* Last basic block */
        {
            basic_blocks.push_back({head_instr[i], head_instr[i]});
            break;
        }
        auto end = head_instr[i];
        for (; end < head_instr[i + 1]; end++)
        {
            if (code[end]->instr == INSTR_GLOB)
                break;
        }
        basic_blocks.push_back({head_instr[i], end});
    }
    return basic_blocks;
}

static std::vector<std::vector<std::size_t>>
find_succ(const std::vector<IntermediateCode*>& code)
{
    std::vector<std::vector<std::size_t>> succs;
    auto basic_blocks = find_basic_blocks(code);
    std::map<std::size_t, std::size_t> index_dict;
    /* 'index_dict' maps label number to basic block number. */
    auto num_blocks = basic_blocks.size();
    for (std::size_t i = 0; i < num_blocks; i++)
    {
        auto block = basic_blocks[i];
        if (i != num_blocks - 1) /* Not EXIT block */
        {
            auto label = code[block.first]->labels;
            if (label.size() > 0)
            {
                index_dict[label[0]] = i; /* Use block number i to mark 
                                    the instruction at the front of block i. */
            }
        }
    }

    for (std::size_t i = 0; i < num_blocks; i++)
    {
        std::vector<std::size_t> successors;
        auto block = basic_blocks[i];
        if (i != num_blocks - 1)
        {
            auto last_instr = code[block.second - 1];
            if (last_instr->instr == INSTR_JE ||
                last_instr->instr == INSTR_JNE)
            {
                successors.push_back(index_dict[last_instr->roperand]);
                successors.push_back(i + 1);
            }
            else if (last_instr->instr == INSTR_JMP)
            {
                successors.push_back(index_dict[last_instr->roperand]);
            }
            else
            {
                successors.push_back(i + 1);
            }
            succs.push_back(successors);
        }
        else
        {
            succs.push_back(std::vector<std::size_t>());
        }
    }
    return succs;
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

std::vector<BasicBlock*> make_basic_blocks(const std::vector<IntermediateCode*>& code)
{
    auto basic_block_idx = find_basic_blocks(code);
    std::vector<BasicBlock*> blocks;
    for (auto& pair: basic_block_idx)
    {
        blocks.push_back(new BasicBlock(make_code_weakcopy(code, pair), pair));
    }
    auto successors = find_succ(code);
    auto length = blocks.size();
    for (std::size_t i = 0; i < length; i++)
    {
        for (auto& j: successors[i])
        {
            blocks[i]->successors.push_back(j);
            blocks[j]->predecessors.push_back(i);
        }
    }
    return blocks;
}

BasicBlock::BasicBlock(const std::vector<IntermediateCode*>& _code, std::pair<std::size_t, std::size_t> _line_range)
{
    code = _code;
    line_range = _line_range;
}

void BasicBlock::print_code()
{
    for (auto& code_line: code)
    {
        printf("%s\n", code_line->to_str().c_str());
    }
}