#include "SmartReference.h"

namespace oly
{
    internal::PoolBatch::PoolBatch()
        : ITickService(TickPhase::PreFrame, TerminatePhase::ReferencePool)
    {
    }

    void internal::PoolBatch::on_tick()
    {
        clean();
    }

    void internal::PoolBatch::on_terminate()
    {
        for (IPool* pool : IPool::tracked())
            pool->clear();

        IPool::tracker().clear();
    }

    void internal::PoolBatch::clean()
    {
        for (IPool* pool : IPool::tracked())
            pool->clean();
    }
}
