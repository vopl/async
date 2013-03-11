#ifndef _ASYNC_SCHEDULER_HPP_
#define _ASYNC_SCHEDULER_HPP_

#include <memory>

namespace async
{
    class ThreadUtilizer;

    class SchedulerImpl;
    typedef std::shared_ptr<SchedulerImpl> SchedulerImplPtr;

    class Scheduler
    {
    public:
        Scheduler();
        Scheduler(const SchedulerImplPtr &schedulerImpl);
        ~Scheduler();

        operator ThreadUtilizer();

    private:
        const SchedulerImplPtr _schedulerImpl;
    };
}

#endif
