#ifndef _ASYNC_IMPL_SYNCHRONIZER_HPP_
#define _ASYNC_IMPL_SYNCHRONIZER_HPP_

#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class Coro;

    class Waiter;

    ////////////////////////////////////////////////////////////////////////////////
    class Synchronizer
    {
    public:
        virtual bool tryAcquire(Waiter *waiter) = 0;
        virtual bool waiterAdd(Waiter *waiter) = 0;
        virtual void waiterDel(Waiter *waiter) = 0;
    };
}}

#endif
