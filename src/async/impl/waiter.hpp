#ifndef _ASYNC_IMPL_WAITER_HPP_
#define _ASYNC_IMPL_WAITER_HPP_

namespace async { namespace impl
{
    class Event;
    class Mutex;

    class Waiter
    {
    public:
        Waiter();
        ~Waiter();
    };

}}

#endif
