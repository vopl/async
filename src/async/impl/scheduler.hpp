#ifndef _ASYNC_IMPL_SCHEDULER_HPP_
#define _ASYNC_IMPL_SCHEDULER_HPP_

#include <memory>
#include "async/threadUtilizer.hpp"
#include "async/impl/threadController.hpp"

namespace async { namespace impl
{
    class Scheduler
        : public std::enable_shared_from_this<Scheduler>
    {
    public:
        Scheduler();
        ~Scheduler();

    public:
        bool te_init(ThreadController *controller);
        void *te_emitWorkPiece();
        void te_deinit();
        EThreadReleaseResult threadRelease(const std::thread::id &id);
    };
}}

#endif
