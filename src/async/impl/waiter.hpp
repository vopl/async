#ifndef _ASYNC_IMPL_WAITER_HPP_
#define _ASYNC_IMPL_WAITER_HPP_

#include "async/impl/synchronizer.hpp"

#include <deque>

namespace async
{
    class Event;
    class Mutex;
}

namespace async { namespace impl
{
    class Waiter
    {
    public:
        Waiter();
        ~Waiter();

        void push(const ::async::Event &waitable);
        void push(const ::async::Mutex &waitable);

        size_t exec();

    private:
        std::deque<SynchronizerPtr> _synchronizers;
    };

}}

#endif
