#ifndef _ASYNC_IMPL_THREADCONTAINER_HPP_
#define _ASYNC_IMPL_THREADCONTAINER_HPP_

#include "async/threadUtilizer.hpp"
#include "async/impl/thread.hpp"

#include <map>

namespace async { namespace impl
{
    class ThreadContainer
    {
    public:
        ThreadContainer();
        ~ThreadContainer();

    public:
        bool te_init(Thread *thread);
        void te_deinit();
        EThreadReleaseResult releaseThread(const std::thread::id &id);
        bool pushWorkPiece(Coro *workPiece);

    private:
        std::mutex _mtx;
        typedef std::map<std::thread::id, Thread *> TMThreads;
        TMThreads _threads;

    };
}}

#endif
