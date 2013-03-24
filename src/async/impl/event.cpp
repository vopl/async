#include "async/impl/event.hpp"

#include <cassert>

namespace async { namespace impl
{
    Event::Event(::async::Event::EResetMode erm, bool initial)
        : _erm(::async::Event::erm_default == erm ? ::async::Event::erm_afterNotifyOne : erm)
        , _state(initial)
    {
    }

    Event::~Event()
    {
    }

    void Event::set(::async::Event::EResetMode erm)
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(::async::Event::erm_default != erm)
        {
            _erm = erm;
        }

        if(_state)
        {
            assert(!waitersAmount());

            switch(_erm)
            {
            default:
            case ::async::Event::erm_default:
                assert(0);
            case ::async::Event::erm_immediately:
                _state = false;
                break;
            case ::async::Event::erm_afterNotifyOne:
            case ::async::Event::erm_afterNotifyAll:
            case ::async::Event::erm_manual:
                break;
            }
            return;
        }

        if(!waitersAmount())
        {
            switch(_erm)
            {
            default:
            case ::async::Event::erm_default:
                assert(0);
            case ::async::Event::erm_immediately:
                break;
            case ::async::Event::erm_afterNotifyOne:
            case ::async::Event::erm_afterNotifyAll:
            case ::async::Event::erm_manual:
                _state = true;
                break;
            }
            return;
        }

        switch(_erm)
        {
        default:
        case ::async::Event::erm_default:
            assert(0);
        case ::async::Event::erm_immediately:
        case ::async::Event::erm_afterNotifyOne:
            l.release();
            notify(1);
            break;
        case ::async::Event::erm_afterNotifyAll:
            l.release();
            notify(-1);
            break;
        case ::async::Event::erm_manual:
            _state = true;
            l.release();
            notify(-1);
            break;
        }
    }

    bool Event::isSet()
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _state;
    }

    void Event::reset()
    {
        std::lock_guard<std::mutex> l(_mtx);
        assert(!waitersAmount());
        _state = false;
    }

    bool Event::waiterAdd(AnyWaiterPtr waiter)
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_state)
        {
            assert(!waitersAmount());

            switch(_erm)
            {
            default:
            case ::async::Event::erm_default:
                assert(0);
            case ::async::Event::erm_immediately:
            case ::async::Event::erm_afterNotifyOne:
            case ::async::Event::erm_afterNotifyAll:
                _state = false;
                l.release();
                notify(1);
                return false;
            case ::async::Event::erm_manual:
                l.release();
                notify(1);
                return false;
            }

            assert(!"never here");
            return false;
        }

        waiterAddInternal(waiter);
        return true;
    }

}}
