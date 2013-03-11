#ifndef _ASYNC_SCHEDULERIMPL_HPP_
#define _ASYNC_SCHEDULERIMPL_HPP_

#include <memory>

namespace async
{
    class SchedulerImpl
        : public std::enable_shared_from_this<SchedulerImpl>
    {
    public:
        SchedulerImpl();
        ~SchedulerImpl();
    };
}

#endif
