#include "async/impl/event.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

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
            coro->scheduler()->coroReadyIfHolded(coro.get());
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
            {
                CoroPtr &coro = _waiters.front();
                coro->scheduler()->coroReadyIfHolded(coro.get());
                coro.reset();
                _waiters.pop_front();
            }
            break;
        case ::async::Event::erm_afterNotifyAll:
            for(CoroPtr &coro : _waiters)
            {
                coro->scheduler()->coroReadyIfHolded(coro.get());
                coro.reset();
            }
            _waiters.clear();
            break;
        case ::async::Event::erm_manual:
            for(CoroPtr &coro : _waiters)
            {
                coro->scheduler()->coroReadyIfHolded(coro.get());
                coro.reset();
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
        Coro *coro = Coro::current();
        assert(coro);
        if(!coro)
        {
            throw !"must be called in async context";
        }

        Scheduler *scheduler = coro->scheduler();

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

            _waiters.push_back(coro->shared_from_this());

            l.release();
            scheduler->contextDeactivate(coro, &_mtx);
        }
    }
}}
