#include "async/impl/waiter.hpp"
#include "async/event.hpp"
#include "async/mutex.hpp"
#include "async/impl/event.hpp"
#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"
#include "async/impl/synchronizerWaiterNode.hpp"

#include <cassert>
#include <iostream>
#include <algorithm>
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

    Waiter::Waiter(impl::SynchronizerWaiterNode *synchronizerWaiterNodes)
        : _state(markActive)
        , _synchronizerWaiterNodes(synchronizerWaiterNodes)
        , _synchronizerWaiterNodesAmount(0)
        , _coro()
    {

    }

    Waiter::Waiter(impl::SynchronizerWaiterNode *synchronizerWaiterNodes, uint32_t synchronizerWaiterNodesAmount)
        : _state(markActive)
        , _synchronizerWaiterNodes(synchronizerWaiterNodes)
        , _synchronizerWaiterNodesAmount(synchronizerWaiterNodesAmount)
        , _coro()
    {
        for(uint32_t i(0); i<_synchronizerWaiterNodesAmount; i++)
        {
//            _synchronizerWaiterNodes[i]._right = 0;
//            _synchronizerWaiterNodes[i]._left = 0;
            _synchronizerWaiterNodes[i]._waiter = this;
            _synchronizerWaiterNodes[i]._synchronizerIndex = i;
        }
    }

    Waiter::~Waiter()
    {
    }


    void Waiter::push(Event *event)
    {
        SynchronizerWaiterNode &node = _synchronizerWaiterNodes[_synchronizerWaiterNodesAmount];
//        node._right = 0;
//        node._left = 0;
        node._waiter = this;
        node._synchronizer._event = event;
        node._synchronizerType = SynchronizerWaiterNode::est_event;
        node._synchronizerIndex = _synchronizerWaiterNodesAmount;

        ++_synchronizerWaiterNodesAmount;
    }

    void Waiter::push(Mutex *mutex)
    {
        SynchronizerWaiterNode &node = _synchronizerWaiterNodes[_synchronizerWaiterNodesAmount];
//        node._right = 0;
//        node._left = 0;
        node._waiter = this;
        node._synchronizer._mutex = mutex;
        node._synchronizerType = SynchronizerWaiterNode::est_mutex;
        node._synchronizerIndex = _synchronizerWaiterNodesAmount;

        ++_synchronizerWaiterNodesAmount;
    }

    uint32_t Waiter::any()
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        _coro = Coro::current();
        assert(_coro);

        static __thread uint32_t seed = 0;
        seed = seed * 1103515245 + (uint32_t)(intptr_t)_coro;
        uint32_t offset = (seed>>16) % _synchronizerWaiterNodesAmount;

        for(uint32_t idxTry(offset); idxTry<_synchronizerWaiterNodesAmount; idxTry++)
        {
            if(_synchronizerWaiterNodes[idxTry].tryAcquire())
            {
                return idxTry;
            }
        }

        for(uint32_t idxTry(0); idxTry<offset; idxTry++)
        {
            if(_synchronizerWaiterNodes[idxTry].tryAcquire())
            {
                return idxTry;
            }
        }

        for(uint32_t idxAdd(0); idxAdd<_synchronizerWaiterNodesAmount; idxAdd++)
        {
            if(!_synchronizerWaiterNodes[idxAdd].waiterAdd(_synchronizerWaiterNodes[idxAdd]))
            {
                for(uint32_t idxDel(0); idxDel<idxAdd; idxDel++)
                {
                    _synchronizerWaiterNodes[idxDel].waiterDel(_synchronizerWaiterNodes[idxDel]);
                }

                uint32_t notified = _state.load();
                while(notified == markPreNotified)
                {
                    //std::this_thread::yield();
                    backoff();
                    notified = _state.load();
                }
                assert(notified < 1024);//1024 - maximum Syncronizers in progress for this waiter instance

                return notified;
            }
        }

        //LOCK l.release();

        uint32_t wasState;
        bool doLoop = true;
        while(doLoop)
        {
            wasState = markActive;
            if(_state.compare_exchange_weak(wasState, markDeactivating))
            {
                Scheduler *scheduler = _coro->scheduler();
                scheduler->contextDeactivate(_coro, &_state, markInactive);
                break;
            }

            switch(wasState)
            {
            case markActive:
                //spurious failure, try one more
                continue;
            case markPreNotified:
                //some Syncronizer fired but not complete interactions, await
                //std::this_thread::yield();
                backoff();
                continue;
            case markDeactivating:
            case markInactive:
                //impossible
                assert(!"impossible");
                abort();
                break;
            default:
                //some Syncronizer fired completely
                assert(wasState < 1024);//1024 - maximum Syncronizers in progress for this waiter instance
                doLoop = false;
                break;
            }
        }



        assert(_state.load() < _synchronizerWaiterNodesAmount);

        for(uint32_t i(0); i<_synchronizerWaiterNodesAmount; i++)
        {
            _synchronizerWaiterNodes[i].waiterDel(_synchronizerWaiterNodes[i]);
        }

        return _state.load();
    }

    Coro *Waiter::getCoro()
    {
        return _coro;
    }

    bool Waiter::notify(impl::SynchronizerWaiterNode &node, bool guaranteeNonInactive)
    {
        assert(this == node._waiter);

        bool coroActive;
        if(guaranteeNonInactive)
        {
            for(;;)
            {
                uint32_t wasState = markActive;
                if(_state.compare_exchange_weak(wasState, markPreNotified))
                {
                    coroActive = true;//markActive == wasState;
                    break;
                }

                switch(wasState)
                {
                case markActive:
                    //await more, spurious fail in _notified.compare_exchange_weak
                    break;
                case markPreNotified:
                    return false;
                case markDeactivating:
                    //context deactivating in progress, impossible
                    //impossible
                    assert(!"impossible");
                    abort();
                    break;
                case markInactive:
                    //context deactivated, impossible
                    assert(!"impossible");
                    abort();
                    break;
                default:
                    assert(wasState < 1024);
                    return false;
                }
            }
        }
        else
        {
            uint32_t wasState = markActive;
            for(;;)
            {
                if(_state.compare_exchange_weak(wasState, markPreNotified))
                {
                    coroActive = markActive == wasState;
                    break;
                }

                switch(wasState)
                {
                case markActive:
                    //await more, spurious fail in _notified.compare_exchange_weak
                    break;
                case markPreNotified:
                    return false;
                case markDeactivating:
                    //context deactivating in progress, await inactive state
                    wasState = markInactive;
                    //std::this_thread::yield();
                    backoff();
                    break;
                case markInactive:
                    //context deactivated, try from inactive state
                    break;
                default:
                    assert(wasState < 1024);
                    return false;
                }
            }
        }

        _state.store(node._synchronizerIndex);

        assert(_coro);
        if(!coroActive)
        {
            Scheduler *scheduler = _coro->scheduler();
            scheduler->coroReady(_coro);
        }

        return true;
    }
}}
