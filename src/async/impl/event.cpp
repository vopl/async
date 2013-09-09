#include "async/impl/event.hpp"
#include "async/impl/waiter.hpp"
#include "async/impl/synchronizerWaiterNode.hpp"

#include <thread>
#include <cassert>
#include <iostream>

namespace async { namespace impl
{
    namespace
    {
//        std::atomic_int bocnt(0);
        void backoff()
        {
//            if(!((++bocnt) % 100))
//            {
//                std::cout<<bocnt<<std::endl;
//            }
            std::this_thread::yield();
        }
    }

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
            assert(empty());
            _state.store(_autoReset ? State::nonsignalled : State::signalled);
            return;
        }

        SynchronizerWaiterNode synchronizerWaiterNodes[1];
        synchronizerWaiterNodes[0]._synchronizer._event = this;
        synchronizerWaiterNodes[0]._synchronizerType = SynchronizerWaiterNode::est_event;

        Waiter waiter(synchronizerWaiterNodes, 1);

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
                //notify waiters by queue, depend on autoReset
                if(_autoReset)
                {
                    //notify only one
                    for(SynchronizerWaiterNode *node(dequeue()); node; node=dequeue())
                    {
                        assert(SynchronizerWaiterNode::est_event == node->_synchronizerType);
                        assert(this == node->_synchronizer._event);
                        if(node->_waiter->notify(*node, false))
                        {
                            //some one notified successfully
                            _state.store(State::nonsignalled);
                            return 1;
                        }
                    }

                    //no one
                    _state.store(State::signalled);
                    return 0;
                }

                //notify all
                size_t notifiedAmount(0);
                for(SynchronizerWaiterNode *node(dequeue()); node; node=dequeue())
                {
                    if(node->_waiter->notify(*node, false))
                    {
                        notifiedAmount++;
                    }
                }

                //all notified
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
                //std::this_thread::yield();
                backoff();
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
                //notify waiters by queue, depend on autoReset
                if(_autoReset)
                {
                    //notify only one
                    for(SynchronizerWaiterNode *node(dequeue()); node; node=dequeue())
                    {
                        if(node->_waiter->notify(*node, false))
                        {
                            //some one notified successfully
                            _state.store(State::nonsignalled);
                            return 1;
                        }
                    }

                    //no one
                    _state.store(State::nonsignalled);
                    return 0;
                }

                //notify all
                size_t notifiedAmount(0);
                for(SynchronizerWaiterNode *node(dequeue()); node; node=dequeue())
                {
                    if(node->_waiter->notify(*node, false))
                    {
                        notifiedAmount++;
                    }
                }

                //all notified
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
                //std::this_thread::yield();
                backoff();
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

    bool Event::tryAcquire()
    {
        if(_autoReset)
        {
            State was = State::signalled;
            if(_state.compare_exchange_strong(was, State::nonsignalled))
            {
                return true;
            }
        }
        else
        {
            if(State::signalled == _state.load())
            {
                return true;
            }
        }

        return false;
    }

    bool Event::waiterAdd(SynchronizerWaiterNode &node)
    {
        assert(SynchronizerWaiterNode::est_event == node._synchronizerType);
        assert(this == node._synchronizer._event);

        for(;;)
        {
            State was = State::signalled;
            if(_state.compare_exchange_weak(was, State::busy))
            {
                assert(empty());
                if(node._waiter->notify(node, true))
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
                //std::this_thread::yield();
                backoff();
                continue;
            case State::nonsignalled:
                //already locked by over waiter, put to waiters queue
                if(_state.compare_exchange_strong(was, State::busy))
                {
                    enqueue(node);

                    //keep locked for over waiter
                    _state.store(State::nonsignalled);
                    return true;
                }

                //waiter is not queued yet, try one more
                continue;
            default:
                assert(!"unknown event state");
                break;
            }
        }

        assert(!"never here");
        return false;
    }

    void Event::waiterDel(SynchronizerWaiterNode &node)
    {
        assert(SynchronizerWaiterNode::est_event == node._synchronizerType);
        assert(this == node._synchronizer._event);

        State was;

        for(;;)
        {
            was = State::nonsignalled;
            if(_state.compare_exchange_weak(was, State::busy))
            {
                //busy mode enabled, stop loop, remove waiter from queue
                break;
            }

            if(State::busy == was)
            {
                //busy by 3rd side, wait
                //std::this_thread::yield();
                backoff();
                continue;
            }

            if(State::nonsignalled == was)
            {
                //spurious failure in CAS, try one more
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

        if(node.queued())
        {
            remove(node);
        }

        assert(State::busy == _state.load());
        assert(State::busy != was);
        _state.store(was);
    }


}}
