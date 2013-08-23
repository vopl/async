#include "async/multiWaiter.hpp"
#include "async/impl/multiWaiter.hpp"

#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include "async/event.hpp"
#include "async/impl/event.hpp"

#include <cassert>

namespace async
{
    MultiWaiter::~MultiWaiter()
    {
        Base::dtor();
    }

    uint32_t MultiWaiter::waitAny()
    {
        return impl().waitAny();
    }

    void MultiWaiter::waitAll()
    {

    }

    void MultiWaiter::ctor(size_t waitablesAmount)
    {
        Base::ctor(waitablesAmount);
    }

    void MultiWaiter::push(Mutex &mutex)
    {
        impl().push(&mutex.impl());
    }

    void MultiWaiter::push(Event &event)
    {
        impl().push(&event.impl());
    }

}
