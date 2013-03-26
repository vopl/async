#include "async/event.hpp"
#include "async/impl/event.hpp"

#include <cassert>

namespace async
{
    Event::Event(bool autoReset)
        : _implEvent(new impl::Event(autoReset))
    {
    }

    Event::Event(const Event &other)
        : _implEvent(other._implEvent)
    {
    }

    Event::Event(Event &&other)
        : _implEvent(std::move(other._implEvent))
    {
    }

    Event::~Event()
    {
    }

    Event &Event::operator=(const Event &other)
    {
        _implEvent = other._implEvent;
        return *this;
    }

    Event &Event::operator=(Event &&other)
    {
        Event(std::move(other)).swap(*this);
        return *this;
    }

    void Event::swap(Event &other)
    {
        _implEvent.swap(other._implEvent);
    }

    size_t Event::set()
    {
        return _implEvent->set();
    }

    size_t Event::pulse()
    {
        return _implEvent->pulse();
    }

    bool Event::isSet()
    {
        return _implEvent->isSet();
    }

    bool Event::reset()
    {
        return _implEvent->reset();
    }
}
