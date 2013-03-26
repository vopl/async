#ifndef _ASYNC_IMPL_SYNCHRONIZER_HPP_
#define _ASYNC_IMPL_SYNCHRONIZER_HPP_

#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class Coro;
    typedef std::shared_ptr<Coro> CoroPtr;

    class AnyWaiter;
    typedef std::shared_ptr<AnyWaiter> AnyWaiterPtr;

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
        friend class ::async::impl::AnyWaiter;
        virtual bool waiterAdd(AnyWaiterPtr waiter);
        void waiterDel(AnyWaiterPtr waiter);

    protected:
        //from waiterAdd
        void waiterAddInternal(AnyWaiterPtr waiter);

    protected:
        size_t waitersAmount();

        size_t notify(size_t waitersAmount=1);
        CoroPtr notifyOneAndGetCoro();

    private:
        std::deque<AnyWaiterPtr> _waiters;
    };
    typedef std::shared_ptr<Synchronizer> SynchronizerPtr;
}}

#endif
