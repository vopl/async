#ifndef _ASYNC_IMPL_SCHEDULER_HPP_
#define _ASYNC_IMPL_SCHEDULER_HPP_

#include "async/impl/threadContainer.hpp"
#include "async/impl/coroContainer.hpp"
#include "async/impl/contextEngine.hpp"

#include <memory>

namespace async { namespace impl
{
    class Scheduler
        : public std::enable_shared_from_this<Scheduler>
        , public ThreadContainer
        , public CoroContainer
        , public ContextEngine
    {
    public:
        Scheduler();
        ~Scheduler();

    public://for thread
        bool te_init(Thread *thread);
        void te_deinit();
    };

    typedef std::shared_ptr<Scheduler> SchedulerPtr;
}}

#endif
