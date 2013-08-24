#ifndef _ASYNC_IMPL_SYNCHRONIZER_HPP_
#define _ASYNC_IMPL_SYNCHRONIZER_HPP_

#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class Coro;
    typedef std::shared_ptr<Coro> CoroPtr;

    class MultiWaiter;

    ////////////////////////////////////////////////////////////////////////////////
    class Synchronizer
    {
    protected:
        Synchronizer();
        virtual ~Synchronizer();

    protected:
        std::mutex _mtx;

    private:
        //from SynchronizerWaiter
        friend class ::async::impl::MultiWaiter;
        virtual bool waiterAdd(MultiWaiter *waiter);
        void waiterDel(MultiWaiter *waiter);

    protected:
        //from waiterAdd
        void waiterAddInternal(MultiWaiter *waiter);

    protected:
        size_t waitersAmount();

        size_t notify(size_t waitersAmount=1);
        Coro *notifyOneAndGetCoro();

    private:
        std::deque<MultiWaiter *> _waiters;
    };
}}

#endif
