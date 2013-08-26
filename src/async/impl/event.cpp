#include "async/impl/event.hpp"
#include "async/impl/multiWaiter.hpp"

#include <cassert>

namespace async { namespace impl
{
    Event::Event(bool autoReset)
        : _autoReset(autoReset)
        , _state(false)
    {
    }

    Event::~Event()
    {
    }

    size_t Event::set()
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_state)
        {
            assert(!waitersAmount());
            return 0;
        }

        if(!waitersAmount())
        {
            _state = true;
            return 0;
        }

        if(_autoReset)
        {
            l.release();
            return notifyOne()?1:0;
        }

        _state = true;
        l.release();
        return notifyAll();
    }

    size_t Event::pulse()
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_state)
        {
            assert(!waitersAmount());
            _state = false;
            return 0;
        }

        if(!waitersAmount())
        {
            return 0;
        }

        if(_autoReset)
        {
            l.release();
            return notifyAll();
        }

        l.release();
        return notifyAll();
    }

    bool Event::isSet()
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _state;
    }

    bool Event::reset()
    {
        std::lock_guard<std::mutex> l(_mtx);

        bool previous = _state;
        if(previous)
        {
            assert(!waitersAmount());
            _state = false;
        }
        return previous;
    }

    bool Event::waiterAdd(MultiWaiter *waiter)
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_state)
        {
            assert(!waitersAmount());

            if(waiter->notify(this, true))
            {
                if(_autoReset)
                {
                    _state = false;
                }
            }

            return false;
        }

        waiterAddInternal(waiter);
        return true;
    }

}}
