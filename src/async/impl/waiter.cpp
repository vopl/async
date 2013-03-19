#include "async/impl/waiter.hpp"
#include "async/event.hpp"

#include <cassert>

namespace async { namespace impl
{
    Waiter *waiterAlloc()
    {
        return new Waiter;
    }

    void waiterPush(Waiter *waiter, const ::async::Event &waitable)
    {
        assert(0);
    }

//    void waiterCollect(Waiter *waiter, const ::async::Mutex &waitable)
//    {
//        assert(0);
//    }

    size_t waiterExec(Waiter *waiter)
    {
        assert(0);
        delete waiter;
        return 0;
    }


    Waiter::Waiter()
    {

    }

    Waiter::~Waiter()
    {

    }

}}
