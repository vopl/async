#ifndef _ASYNC_IMPL_SCHEDULER_HPP_
#define _ASYNC_IMPL_SCHEDULER_HPP_

#include "async/impl/threadControllerSet.hpp"

#include <memory>

namespace async { namespace impl
{
    class Scheduler
        : public std::enable_shared_from_this<Scheduler>
        , public ThreadControllerSet
    {
    public:
        Scheduler();
        ~Scheduler();
    };
}}

#endif
