#include "async/impl/event.hpp"

#include <cassert>

namespace async { namespace impl
{
    Event::Event(::async::Event::EResetMode erm, bool initial)
        : _erm_(::async::Event::erm_default == erm ? ::async::Event::erm_afterNotifyOne : erm)
        , _state(initial)
    {
    }

    Event::~Event()
    {
        assert(_waiters.empty());
        for(const CoroPtr &coro : _waiters)
        {
            coro->readyIfHolded();
        }
    }

    void Event::set(::async::Event::EResetMode erm)
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(::async::Event::erm_default == erm)
        {
            erm = _erm_;
        }

        if(_state)
        {
            assert(_waiters.empty());

            switch(erm)
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

        if(_waiters.empty())
        {
            switch(erm)
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

        switch(erm)
        {
        default:
        case ::async::Event::erm_default:
            assert(0);
        case ::async::Event::erm_immediately:
        case ::async::Event::erm_afterNotifyOne:
            _waiters.front()->readyIfHolded();
            _waiters.pop_front();
            break;
        case ::async::Event::erm_afterNotifyAll:
            for(const CoroPtr &coro : _waiters)
            {
                coro->readyIfHolded();
            }
            _waiters.clear();
            break;
        case ::async::Event::erm_manual:
            for(const CoroPtr &coro : _waiters)
            {
                coro->readyIfHolded();
            }
            _waiters.clear();
            _state = true;
            break;
        }
    }

    bool Event::isSet()
    {
        std::unique_lock<std::mutex> l(_mtx);
        return _state;
    }

    void Event::reset()
    {
        std::unique_lock<std::mutex> l(_mtx);
        assert(_waiters.empty());
        _state = false;
    }

    void Event::wait()
    {
        Coro *coro;
        {
            std::unique_lock<std::mutex> l(_mtx);

            assert(_waiters.size() < 200);
            if(_state)
            {
                switch(_erm_)
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

            coro = Coro::current();
            assert(coro);
            if(!coro)
            {
                throw !"must be called in async context";
            }
            _waiters.push_back(coro->shared_from_this());
        }

        //TODO this must be atomic with Event::wait
        coro->hold();
    }
}}
