#include "async/details/multiWaiter.hpp"
#include "async/impl/multiWaiter.hpp"

#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include "async/event.hpp"
#include "async/impl/event.hpp"

#include <cassert>

namespace async { namespace details
{
    MultiWaiter::MultiWaiter(impl::Synchronizer **synchronizersBuffer)
        : Base(synchronizersBuffer)
    {
    }

    MultiWaiter::~MultiWaiter()
    {
    }

    uint32_t MultiWaiter::waitAny()
    {
        return impl().waitAny();
    }

    void MultiWaiter::waitAll()
    {
        assert(!"not impl yet");
    }

    void MultiWaiter::push(Mutex &mutex)
    {
        impl().push(&mutex.impl());
    }

    void MultiWaiter::push(Event &event)
    {
        impl().push(&event.impl());
    }
}}
