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
        waiter->push(waitable);
    }

//    void waiterPush(Waiter *waiter, const ::async::Mutex &waitable)
//    {
//        waiter->push(waitable);
//    }

    size_t waiterExec(Waiter *waiter)
    {
        size_t result = waiter->exec();
        delete waiter;
        return result;
    }


    Waiter::Waiter()
    {

    }

    Waiter::~Waiter()
    {

    }

    void Waiter::push(const ::async::Event &waitable)
    {
        const EventPtr &implPtr = waitable._implEvent;
        assert(0);
    }

//    void Waiter::push(const ::async::Mutex &waitable)
//    {
//    }

    size_t Waiter::exec()
    {
        assert(0);
    }

}}
