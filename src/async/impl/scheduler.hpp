#ifndef _ASYNC_IMPL_SCHEDULER_HPP_
#define _ASYNC_IMPL_SCHEDULER_HPP_

#include "async/impl/threadContainer.hpp"
#include "async/impl/contextContainer.hpp"

#include <memory>

namespace async { namespace impl
{
    class Scheduler
        : public std::enable_shared_from_this<Scheduler>
        , public ThreadContainer
        , public ContextContainer
    {
    public:
        Scheduler();
        ~Scheduler();
    };

    typedef std::shared_ptr<Scheduler> SchedulerPtr;
}}

#endif
