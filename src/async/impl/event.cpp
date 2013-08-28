#include "async/impl/event.hpp"
#include "async/impl/waiter.hpp"

#include <thread>
#include <cassert>

namespace async { namespace impl
{
    Event::Event(bool autoReset)
        : _autoReset(autoReset)
        , _state(State::nonsignalled)
    {
    }

    Event::~Event()
    {
    }

    void Event::wait()
    {
        State was = State::signalled;
        if(_state.compare_exchange_strong(was, State::busy))
        {
            assert(_waiters.empty());
            _state.store(_autoReset ? State::nonsignalled : State::signalled);
            return;
        }

        Synchronizer *synchronizersBuffer[1] = {this};
        Waiter waiter(synchronizersBuffer, 1);

        uint32_t waiterResult = waiter.any();

        (void)waiterResult;
        assert(0 == waiterResult);

        return;
    }

    size_t Event::set()
    {
        for(;;)
        {
            State was = State::nonsignalled;

            if(_state.compare_exchange_strong(was, State::busy))
            {
                //wakeup next if exists
                if(_waiters.empty())
                {
                    //nobody, keep signalled
                    _state.store(State::signalled);
                    return 0;
                }

                //notify waiters by queue, depend on autoReset
                TVWaiters::iterator iter(_waiters.begin());
                TVWaiters::iterator end(_waiters.end());

                if(_autoReset)
                {
                    //notify only first
                    for(; iter != end; ++iter)
                    {
                        Waiter *waiter = *iter;
                        if(waiter->notify(this, false))
                        {
                            ++iter;

                            //some one notified successfully
                            _waiters.erase(_waiters.begin(), iter);
                            _state.store(State::nonsignalled);
                            return 1;
                        }
                    }

                    //no one
                    _waiters.clear();
                    _state.store(State::signalled);
                    return 0;
                }

                //notify all
                size_t notifiedAmount(0);
                for(; iter != end; ++iter)
                {
                    Waiter *waiter = *iter;
                    if(waiter->notify(this, false))
                    {
                        ++notifiedAmount;
                    }
                }

                //all notified
                _waiters.clear();
                _state.store(State::signalled);
                return notifiedAmount;
            }

            switch(was)
            {
            case State::signalled:
                //already signalled
                return 0;
            case State::nonsignalled:
                //spurious failure, try one more
                continue;
            case State::busy:
                //over thread make changes, wait
                std::this_thread::yield();
                continue;
            default:
                assert(!"unknown mutex state");
                break;
            }
        }

        assert(!"never here");
        return 0;
    }

    size_t Event::pulse()
    {
        for(;;)
        {
            State was = State::nonsignalled;

            if(_state.compare_exchange_strong(was, State::busy))
            {
                //wakeup next if exists
                if(_waiters.empty())
                {
                    //nobody, keep nonsignalled
                    _state.store(State::nonsignalled);
                    return 0;
                }

                //notify waiters by queue, depend on autoReset
                TVWaiters::iterator iter(_waiters.begin());
                TVWaiters::iterator end(_waiters.end());

                if(_autoReset)
                {
                    //notify only first
                    for(; iter != end; ++iter)
                    {
                        Waiter *waiter = *iter;
                        if(waiter->notify(this, false))
                        {
                            ++iter;

                            //some one notified successfully
                            _waiters.erase(_waiters.begin(), iter);
                            _state.store(State::nonsignalled);
                            return 1;
                        }
                    }

                    //no one
                    _waiters.clear();
                    _state.store(State::nonsignalled);
                    return 0;
                }

                //notify all
                size_t notifiedAmount(0);
                for(; iter != end; ++iter)
                {
                    Waiter *waiter = *iter;
                    if(waiter->notify(this, false))
                    {
                        ++notifiedAmount;
                    }
                }

                //all notified
                _waiters.clear();
                _state.store(State::nonsignalled);
                return notifiedAmount;
            }

            switch(was)
            {
            case State::signalled:
                //already signalled, keep
                return 0;
            case State::nonsignalled:
                //spurious failure, try one more
                continue;
            case State::busy:
                //over thread make changes, wait
                std::this_thread::yield();
                continue;
            default:
                assert(!"unknown mutex state");
                break;
            }
        }

        assert(!"never here");
        return 0;
    }

    bool Event::isSet()
    {
        switch(_state.load(/*relaxed?*/))
        {
        case State::signalled:
            return true;
        default:
            break;
        }

        return false;
    }

    bool Event::reset()
    {
        for(;;)
        {
            State was = State::signalled;

            if(_state.compare_exchange_strong(was, State::nonsignalled))
            {
                return true;
            }

            switch (was)
            {
            case State::nonsignalled:
                //already nonsignalled
                return false;
            case State::busy:
                //wait unbusy
                continue;
            default:
                assert(0);
                break;
            }
        }

        assert(!"never here");
        return false;
    }

    bool Event::waiterAdd(Waiter *waiter)
    {
        for(;;)
        {
            State was = State::signalled;
            if(_state.compare_exchange_weak(was, State::busy))
            {
                assert(_waiters.empty());
                if(waiter->notify(this, true))
                {
                    _state.store(_autoReset ? State::nonsignalled : State::signalled);
                }
                else
                {
                    //waiter already notified by 3rd side, keep signalled state
                    _state.store(State::signalled);
                }
                return false;
            }

            switch(was)
            {
            case State::signalled:
                //spurious failure, try one more
                continue;
            case State::busy:
                //over thread make changes, wait
                std::this_thread::yield();
                continue;
            case State::nonsignalled:
                //already locked by over waiter, put to waiters queue
                if(_state.compare_exchange_strong(was, State::busy))
                {
                    _waiters.push_back(waiter);

                    //keep locked for over waiter
                    _state.store(State::nonsignalled);
                    return true;
                }

                //waiter is not queued yet, try one more
                continue;
            default:
                assert(!"unknown mutex state");
                break;
            }
        }

        assert(!"never here");
        return false;
    }

    void Event::waiterDel(Waiter *waiter)
    {
        State was;

        for(;;)
        {
            was = State::nonsignalled;
            if(_state.compare_exchange_weak(was, State::busy))
            {
                //busy mode enabled, stop loop, remove waiter from queue
                break;
            }

            if(State::nonsignalled == was)
            {
                //spuriout failure in CAS, try one more
                continue;
            }

            if(State::busy == was)
            {
                //busy by 3rd side, wait
                std::this_thread::yield();
                continue;
            }

            assert(State::signalled == was);

            //locked by over waiter, do local busy-lock from him
            if(_state.compare_exchange_strong(was, State::busy))
            {
                //busy mode enabled, stop loop, remove waiter from queue, original lock state preserved in _was_
                break;
            }

            //waiter is not queued yet, try one more
            continue;
        }
        //note, original lock state preserved in _was_

        for(std::vector<Waiter *>::iterator iter(_waiters.begin()), end(_waiters.end()); iter != end; ++iter)
        {
            if(waiter == *iter)
            {
                _waiters.erase(iter);
                break;
            }
        }

        assert(State::busy == _state.load());
        assert(State::busy != was);
        _state.store(was);
    }


}}
