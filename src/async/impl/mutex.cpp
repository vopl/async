#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/waiter.hpp"
#include "async/impl/synchronizerWaiterNode.hpp"

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
        assert(State::unlocked == _state && empty());
    }

    void Mutex::lock()
    {
        if(tryLock())
        {
            return;
        }

        SynchronizerWaiterNode synchronizerWaiterNodes[1];

        synchronizerWaiterNodes[0]._synchronizer._mutex = this;
        synchronizerWaiterNodes[0]._synchronizerType = SynchronizerWaiterNode::est_mutex;

        Waiter waiter(synchronizerWaiterNodes, 1);

        uint32_t waiterResult = waiter.any();

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
        for(;;)
        {
            State was = State::locked;

            if(_state.compare_exchange_strong(was, State::busy))
            {
                //wakeup next if exists
                if(empty())
                {
                    //nobody
                    _state.store(State::unlocked);
                    return;
                }

                //notify waiters by queue, drop all notified, who accept notifications - stand locker
                for(SynchronizerWaiterNode *node(dequeue()); node; node=dequeue())
                {
                    if(node->_waiter->notify(*node, false))
                    {
                        //some one notified successfully
                        _state.store(State::locked);
                        return;
                    }
                }

                //no one
                _state.store(State::unlocked);
                return;
            }

            switch(was)
            {
            case State::unlocked:
                assert(!"unable to unlock already unlocked mutex");
                return;
            case State::locked:
                //spurious failure, try one more
                continue;
            case State::busy:
                //over thread make changes, wait
                //std::this_thread::yield();
                continue;
            default:
                assert(!"unknown mutex state");
                break;
            }
        }
    }

    bool Mutex::waiterAdd(SynchronizerWaiterNode &node)
    {
        assert(SynchronizerWaiterNode::est_mutex == node._synchronizerType);
        assert(this == node._synchronizer._mutex);

        for(;;)
        {
            State was = State::unlocked;
            if(_state.compare_exchange_weak(was, State::busy))
            {
                assert(empty());
                if(node._waiter->notify(node, true))
                {
                    //waiter notified successfully, his come locker
                    _state.store(State::locked);
                }
                else
                {
                    //waiter already notified by 3rd side
                    _state.store(State::unlocked);
                }
                return false;
            }

            switch(was)
            {
            case State::unlocked:
                assert(empty());
                //spurious failure, try one more
                continue;
            case State::busy:
                //over thread make changes, wait
                //std::this_thread::yield();
                continue;
            case State::locked:
                //already locked by over waiter, put to waiters queue
                if(_state.compare_exchange_strong(was, State::busy))
                {
                    enqueue(node);

                    //keep locked for over waiter
                    _state.store(State::locked);
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

    void Mutex::waiterDel(SynchronizerWaiterNode &node)
    {
        assert(SynchronizerWaiterNode::est_mutex == node._synchronizerType);
        assert(this == node._synchronizer._mutex);

        State was;

        for(;;)
        {
            was = State::unlocked;
            if(_state.compare_exchange_weak(was, State::busy))
            {
                //busy mode enabled, stop loop, remove waiter from queue
                break;
            }

            if(State::unlocked == was)
            {
                //spurious failure in CAS, try one more
                continue;
            }

            if(State::busy == was)
            {
                //std::this_thread::yield();
                continue;
            }

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

        remove(node);

        assert(State::busy == _state.load());
        assert(State::busy != was);
        _state.store(was);
    }


}}
