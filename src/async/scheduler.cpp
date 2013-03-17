#include "async/stable.hpp"
#include "async/scheduler.hpp"
#include "async/impl/scheduler.hpp"
#include "async/threadUtilizer.hpp"
#include "async/codeManager.hpp"

#include <cassert>

namespace async
{
    Scheduler::Scheduler()
        : _implScheduler(new impl::Scheduler)
    {
    }

    Scheduler::Scheduler(const impl::SchedulerPtr &implScheduler)
        : _implScheduler(implScheduler)
    {

    }

    Scheduler::~Scheduler()
    {
    }

    Scheduler::operator ThreadUtilizer()
    {
        return ThreadUtilizer(_implScheduler);
    }

    Scheduler::operator CodeManager()
    {
        return CodeManager(_implScheduler);
    }

}
