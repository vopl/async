#ifndef _ASYNC_SCHEDULER_HPP_
#define _ASYNC_SCHEDULER_HPP_

#include <memory>

namespace async
{
    class ThreadUtilizer;

    namespace impl
    {
        class Scheduler;
        typedef std::shared_ptr<Scheduler> SchedulerPtr;
    }

    class Scheduler
    {
    public:
        Scheduler();
        Scheduler(const impl::SchedulerPtr &implScheduler);
        ~Scheduler();

        operator ThreadUtilizer();

    private:
        const impl::SchedulerPtr _implScheduler;
    };
}

#endif
