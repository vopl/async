#include "async/impl/scheduler.hpp"
#include <cassert>

namespace async { namespace impl
{
    Scheduler::Scheduler()
    {
    }

    Scheduler::~Scheduler()
    {
    }

    bool Scheduler::te_init(ThreadController *controller)
    {
        assert(0);
        return true;
    }

    void *Scheduler::te_emitWorkPiece()
    {
        assert(0);
        return this;
    }

    void Scheduler::te_deinit()
    {
        assert(0);
    }

    EThreadReleaseResult Scheduler::threadRelease(const std::thread::id &id)
    {
        assert(0);
        return etrr_notInWork;
    }

}}
