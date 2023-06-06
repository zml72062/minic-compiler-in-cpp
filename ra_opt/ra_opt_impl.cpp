#include "basicblock.h"

void register_alloc_optim(std::vector<IntermediateCode*>& code)
{
    /* Decompose code into procedures (functions). */
    auto proc = make_procedures(code);
    for (auto& p: proc)
    {
        /* Decompose procedures into basic blocks. */
        auto blocks = make_basic_blocks(p->code);

        /* Liveness analysis. */
        LivenessUpdater updater(blocks);
        updater.calculate_liveness();

        /* Register allocation. */
        RegisterAllocator alloc(AllocationTable(p->register_range()));
        alloc.allocate(updater);

        for (auto & b: blocks)
        {
            delete b;
        }
    }
    for (auto& p: proc)
    {
        delete p;
    }
    MemorySpiller().spill(code);
    remove_useless_mov(code);
}