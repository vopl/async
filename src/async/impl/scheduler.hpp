#ifndef _ASYNC_IMPL_SCHEDULER_HPP_
#define _ASYNC_IMPL_SCHEDULER_HPP_

#include <memory>

namespace async { namespace impl
{
    class Scheduler
        : public std::enable_shared_from_this<Scheduler>
    {
    public:
        Scheduler();
        ~Scheduler();
    };
}}

#endif
