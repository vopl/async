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
            return notify(1);
        }

        _state = true;
        l.release();
        return notify(-1);
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
            return notify(-1);
        }

        l.release();
        return notify(-1);
    }

    bool Event::isSet()
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _state;
    }

    bool Event::reset()
    {
        std::lock_guard<std::mutex> l(_mtx);
        assert(!waitersAmount());

        bool previous = _state;
        _state = false;
        return previous;
    }

    bool Event::waiterAdd(MultiWaiter *waiter)
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_state)
        {
            assert(!waitersAmount());

            if(waiter->notify(this))
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
