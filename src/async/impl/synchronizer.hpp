#ifndef _ASYNC_IMPL_SYNCHRONIZER_HPP_
#define _ASYNC_IMPL_SYNCHRONIZER_HPP_

#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class Coro;

    class MultiWaiter;

    ////////////////////////////////////////////////////////////////////////////////
    class Synchronizer
    {
    public:
        virtual bool waiterAdd(MultiWaiter *waiter) = 0;
        virtual void waiterDel(MultiWaiter *waiter) = 0;
    };
}}

#endif
