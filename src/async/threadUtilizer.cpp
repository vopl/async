#include "async/threadUtilizer.hpp"
#include "async/impl/scheduler.hpp"
#include <cassert>

#include <mutex>

namespace async
{
    ThreadUtilizer::ThreadUtilizer()
        : _implScheduler(new impl::Scheduler)
    {
    }

    ThreadUtilizer::ThreadUtilizer(const impl::SchedulerPtr &implScheduler)
        : _implScheduler(implScheduler)
    {
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize()
    {
        assert(0);
        return etur_alreadyInWork;
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const _clock::duration &duration)
    {
        assert(0);
        return etur_alreadyInWork;
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const _clock::time_point &atime)
    {
        assert(0);
        return etur_alreadyInWork;
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const size_t &workPiecesAmount)
    {
        assert(0);
        return etur_alreadyInWork;
    }

    EThreadReleaseResult ThreadUtilizer::te_release()
    {
        assert(0);
        return etrr_notInWork;
    }

    EThreadReleaseResult ThreadUtilizer::release(std::thread::native_handle_type id)
    {
        assert(0);
        return etrr_notInWork;
    }

}


