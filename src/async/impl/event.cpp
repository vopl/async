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
        std::lock_guard<std::mutex> l(_mtx);

        if(::async::Event::erm_default != erm)
        {
            _erm = erm;
        }

        if(_state)
        {
            assert(!holdsAmount());

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

        if(!holdsAmount())
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
            activateOthers(1);
            break;
        case ::async::Event::erm_afterNotifyAll:
            activateOthers(-1);
            break;
        case ::async::Event::erm_manual:
            activateOthers(-1);
            _state = true;
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
        assert(!holdsAmount());
        _state = false;
    }

    void Event::wait()
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_state)
        {
            switch(_erm)
            {
            default:
            case ::async::Event::erm_default:
                assert(0);
            case ::async::Event::erm_afterNotifyOne:
            case ::async::Event::erm_immediately:
            case ::async::Event::erm_afterNotifyAll:
                _state = false;
                break;
            case ::async::Event::erm_manual:
                break;
            }
            return;
        }

        l.release();
        holdSelf();
    }
}}
