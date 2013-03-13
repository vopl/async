#ifndef _ASYNC_IMPL_THREADCONTAINER_HPP_
#define _ASYNC_IMPL_THREADCONTAINER_HPP_

#include "async/threadUtilizer.hpp"
#include "async/impl/threadController.hpp"
#include "async/impl/context.hpp"

#include <map>

namespace async { namespace impl
{
    class ThreadContainer
    {
    public:
        ThreadContainer();
        ~ThreadContainer();

    public:
        bool te_init(ThreadController *controller);
        ContextPtr te_emitWorkPiece();
        void te_deinit();
        EThreadReleaseResult release(const std::thread::id &id);

    private:
        std::mutex _mtx;
        typedef std::map<std::thread::id, ThreadController *> TMThreads;
        TMThreads _threads;

    };
}}

#endif
