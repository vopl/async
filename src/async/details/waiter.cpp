#include "async/details/waiter.hpp"
#include "async/impl/waiter.hpp"

#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include "async/event.hpp"
#include "async/impl/event.hpp"

#include <cassert>

namespace async { namespace details
{
    Waiter::Waiter(impl::SynchronizerWaiterNode *synchronizerWaiterNodes)
        : Base(synchronizerWaiterNodes)
    {
    }

    Waiter::~Waiter()
    {
    }

    uint32_t Waiter::any()
    {
        return impl().any();
    }

    void Waiter::all()
    {
        assert(!"not impl yet");
    }

    void Waiter::push(Mutex &mutex)
    {
        impl().push(&mutex.impl());
    }

    void Waiter::push(Event &event)
    {
        impl().push(&event.impl());
    }
}}
