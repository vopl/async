#include "async/event.hpp"
#include "async/impl/event.hpp"

#include <cassert>

namespace async
{
    Event::Event(bool autoReset)
        : Base(autoReset)
    {
    }

    Event::~Event()
    {
    }

    size_t Event::set()
    {
        return impl().set();
    }

    size_t Event::pulse()
    {
        return impl().pulse();
    }

    bool Event::isSet()
    {
        return impl().isSet();
    }

    bool Event::reset()
    {
        return impl().reset();
    }
}
