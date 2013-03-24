#include "async/event.hpp"
#include "async/impl/event.hpp"

#include <cassert>

namespace async
{
    Event::Event(EResetMode erm, bool initial)
        : _implEvent(new impl::Event(erm, initial))
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

    void Event::set(EResetMode erm)
    {
        return _implEvent->set(erm);
    }

    bool Event::isSet()
    {
        return _implEvent->isSet();
    }

    void Event::reset()
    {
        return _implEvent->reset();
    }
}
