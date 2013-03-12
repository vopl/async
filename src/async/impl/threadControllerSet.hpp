#ifndef _ASYNC_IMPL_THREADCONTROLLERSET_HPP_
#define _ASYNC_IMPL_THREADCONTROLLERSET_HPP_

#include "async/threadUtilizer.hpp"
#include "async/impl/threadController.hpp"

#include <map>

namespace async { namespace impl
{
    class ThreadControllerSet
    {
    public:
        ThreadControllerSet();
        ~ThreadControllerSet();

    public:
        bool te_init(ThreadController *controller);
        void *te_emitWorkPiece();
        void te_deinit();
        EThreadReleaseResult threadRelease(const std::thread::id &id);

    private:
        std::mutex _mtx;
        typedef std::map<std::thread::id, ThreadController *> TMThreads;
        TMThreads _threads;

    };
}}

#endif
