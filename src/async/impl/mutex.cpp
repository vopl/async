#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/multiWaiter.hpp"

#include <thread>
#include <cassert>
//#include <iostream>

namespace async { namespace impl
{
    Mutex::Mutex()
        : _state(State::unlocked)
    {
    }

    Mutex::~Mutex()
    {
        assert(State::unlocked == _state && _waiters.empty());
    }

    void Mutex::lock()
    {
        if(tryLock())
        {
            return;
        }

        Synchronizer *synchronizersBuffer[1];
        MultiWaiter multiWaiter(synchronizersBuffer);
        multiWaiter.push(this);

        uint32_t waiterResult = multiWaiter.waitAny();

        (void)waiterResult;
        assert(0 == waiterResult);

        return;
    }

    bool Mutex::tryLock()
    {
        State was = State::unlocked;
        if(_state.compare_exchange_strong(was, State::locked))
        {
            return true;
        }

        return false;
    }

    bool Mutex::isLocked()
    {
        switch(_state.load(/*relaxed?*/))
        {
        case State::busy:
        case State::locked:
            return true;
        default:
            break;
        }

        return false;
    }

    void Mutex::unlock()
    {
//        std::cout<<"unlock\n"; std::cout.flush();
        for(;;)
        {
            State was = State::locked;

            if(_state.compare_exchange_strong(was, State::busy))
            {
//                std::cout<<"unlock make busy\n"; std::cout.flush();
                //wakeup next if exists
                if(_waiters.empty())
                {
                    //nobody
                    _state.exchange(State::unlocked);
//                    std::cout<<"unlock make unlocked\n"; std::cout.flush();
                    return;
                }

                //notify waiters by queue, drop all notified, who accept notifications - stand locker
                TVWaiters::iterator iter(_waiters.begin());
                TVWaiters::iterator end(_waiters.end());

                for(; iter != end; ++iter)
                {
                    MultiWaiter *waiter = *iter;
                    if(waiter->notify(this, false))
                    {
                        ++iter;

                        //some one notified successfully
                        _waiters.erase(_waiters.begin(), iter);
                        _state.exchange(State::locked);
//                        std::cout<<"unlock make locked\n"; std::cout.flush();
                        return;
                    }
                }

                //no one
                _waiters.clear();
                _state.exchange(State::unlocked);
//                std::cout<<"unlock make unlocked because nobody\n"; std::cout.flush();
                return;
            }

            switch(was)
            {
            case State::unlocked:
//                std::cout<<"unlock unable busy, unlocked\n"; std::cout.flush();
                assert(!"unable to unlock already unlocked mutex");
                return;
            case State::locked:
//                std::cout<<"unlock unable busy, locked\n"; std::cout.flush();
                //spurious failure, try one more
                continue;
            case State::busy:
//                std::cout<<"unlock unable busy, busy\n"; std::cout.flush();
                //over thread make changes, wait
                std::this_thread::yield();
                continue;
            default:
                assert(!"unknown mutex state");
                break;
            }
        }
    }

    bool Mutex::waiterAdd(MultiWaiter *waiter)
    {
//        std::cout<<"waiterAdd\n"; std::cout.flush();
        for(;;)
        {
            State was = State::unlocked;
            if(_state.compare_exchange_weak(was, State::busy))
            {
//                std::cout<<"waiteAdd, make busy\n"; std::cout.flush();
                assert(_waiters.empty());
                if(waiter->notify(this, true))
                {
                    //waiter already notified by 3rd side
                    _state.exchange(State::locked);
//                    std::cout<<"waiteAdd, make locked\n"; std::cout.flush();
                }
                else
                {
                    //waiter already notified by 3rd side
                    _state.exchange(State::unlocked);
//                    std::cout<<"waiteAdd, make unlocked\n"; std::cout.flush();
                }
                return false;
            }

            switch(was)
            {
            case State::unlocked:
//                std::cout<<"waiteAdd, unable busy, unlocked\n"; std::cout.flush();
                assert(_waiters.empty());
                //spurious failure, try one more
                continue;
            case State::busy:
//                std::cout<<"waiteAdd, unable busy, busy\n"; std::cout.flush();
                //over thread make changes, wait
                std::this_thread::yield();
                continue;
            case State::locked:
//                std::cout<<"waiteAdd, unable busy, locked\n"; std::cout.flush();
                //already locked by over waiter, put to waiters queue
                if(_state.compare_exchange_strong(was, State::busy))
                {
//                    std::cout<<"waiteAdd, locked, busy\n"; std::cout.flush();
                    _waiters.push_back(waiter);

                    //keep locked for over waiter
                    _state.exchange(State::locked);
//                    std::cout<<"waiteAdd, make locked\n"; std::cout.flush();
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

    void Mutex::waiterDel(MultiWaiter *waiter)
    {
//        std::cout<<"waiteDel\n"; std::cout.flush();
        State was;

        for(;;)
        {
            was = State::unlocked;
            if(_state.compare_exchange_weak(was, State::busy))
            {
//                std::cout<<"waiteDel, make busy\n"; std::cout.flush();
                //busy mode enabled, stop loop, remove waiter from queue
                break;
            }

            if(State::unlocked == was)
            {
//                std::cout<<"waiteDel, unable busy, unlocked\n"; std::cout.flush();
                //spuriout failure in CAS, try one more
                continue;
            }

            if(State::busy == was)
            {
//                std::cout<<"waiteDel, unable busy, busy\n"; std::cout.flush();
                std::this_thread::yield();
                continue;
            }

            //locked by over waiter, do local busy-lock from him
            if(_state.compare_exchange_strong(was, State::busy))
            {
//                std::cout<<"waiteDel, unable busy, locked, busy\n"; std::cout.flush();
                //busy mode enabled, stop loop, remove waiter from queue, original lock state preserved in _was_
                break;
            }

            //waiter is not queued yet, try one more
            continue;
        }
        //note, original lock state preserved in _was_

        for(std::vector<MultiWaiter *>::iterator iter(_waiters.begin()), end(_waiters.end()); iter != end; ++iter)
        {
            if(waiter == *iter)
            {
                _waiters.erase(iter);
                break;
            }
        }

        assert(State::busy == _state.load());
        assert(State::busy != was);
        _state.exchange(was);
//        std::cout<<"waiteDel, restore old\n"; std::cout.flush();
    }


}}
