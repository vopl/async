#ifndef _ASYNC_IMPL_SYNCHRONIZER_HPP_
#define _ASYNC_IMPL_SYNCHRONIZER_HPP_

#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class Coro;
    typedef std::shared_ptr<Coro> CoroPtr;

    class Synchronizer
    {
    protected:
        Synchronizer();
        ~Synchronizer();

    protected:
        std::mutex _mtx;

        size_t holdsAmount();
        void activateOthers(size_t holdsAmount=1);
        void holdSelf();

    private:
        std::deque<CoroPtr> _holds;
    };
    typedef std::shared_ptr<Synchronizer> SynchronizerPtr;
}}

#endif
