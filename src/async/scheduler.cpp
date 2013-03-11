#include "async/scheduler.hpp"
#include "async/schedulerImpl.hpp"
#include "async/threadUtilizer.hpp"

#include <cassert>

namespace async
{
    Scheduler::Scheduler()
        : _schedulerImpl(new SchedulerImpl)
    {
    }

    Scheduler::Scheduler(const SchedulerImplPtr &schedulerImpl)
        : _schedulerImpl(schedulerImpl)
    {

    }

    Scheduler::~Scheduler()
    {
    }

    Scheduler::operator ThreadUtilizer()
    {
        return ThreadUtilizer(_schedulerImpl);
    }

}
