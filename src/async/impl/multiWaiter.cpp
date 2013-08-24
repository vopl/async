#include "async/impl/multiWaiter.hpp"
#include "async/event.hpp"
#include "async/mutex.hpp"
#include "async/impl/event.hpp"
#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>
#include <iostream>

namespace async { namespace impl
{
    MultiWaiter::MultiWaiter(Synchronizer **synchronizersBuffer)
        : _synchronizersBuffer(synchronizersBuffer)
        , _synchronizersAmount(0)
        , _state(markActive)
        , _coro()
    {

    }

    MultiWaiter::~MultiWaiter()
    {
    }


    void MultiWaiter::push(Synchronizer *synchronizer)
    {
        _synchronizersBuffer[_synchronizersAmount++] = synchronizer;
    }

    uint32_t MultiWaiter::waitAny()
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        _coro = Coro::current();
        assert(_coro);

        for(uint32_t idxAdd(0); idxAdd<_synchronizersAmount; idxAdd++)
        {
            if(!_synchronizersBuffer[idxAdd]->waiterAdd(this))
            {
                uint32_t notified= _state.load();
                assert(_state.load() <= idxAdd);

                for(uint32_t idxDel(0); idxDel<idxAdd; idxDel++)
                {
                    _synchronizersBuffer[idxDel]->waiterDel(this);
                }

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
                break;
            case markPreNotified:
                //some Syncronizer fired but not complete interactions, await
                std::this_thread::yield();
                break;
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



        //LOCK std::lock_guard<std::mutex> l2(_mtx);
        assert(_state.load() < _synchronizersAmount);


        for(uint32_t i(0); i<_synchronizersAmount; i++)
        {
            _synchronizersBuffer[i]->waiterDel(this);
        }

        return _state.load();
    }

    Coro *MultiWaiter::getCoro()
    {
        return _coro;
    }

    bool MultiWaiter::notify(Synchronizer *notifier, bool guaranteeNonInactive)
    {
        bool coroActive;
        if(guaranteeNonInactive)
        {
            for(;;)
            {
                uint32_t wasState = markActive;
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
                    wasState = markActive;
                    break;
                case markPreNotified:
                    return false;
                case markDeactivating:
                    //context deactivating in progress, await inactive state
                    wasState = markInactive;
                    std::this_thread::yield();
                    break;
                case markInactive:
                    //context deactivated, try from inactive state
                    wasState = markInactive;
                    break;
                default:
                    assert(wasState < 1024);
                    return false;
                }
            }
        }

        for(uint32_t i(0); i<_synchronizersAmount; i++)
        {
            if(notifier == _synchronizersBuffer[i])
            {
                _state.store(i);
            }
        }
        assert(_state.load() < _synchronizersAmount);


        assert(_coro);
        if(!coroActive)
        {
            Scheduler *scheduler = _coro->scheduler();
            scheduler->coroReadyIfHolded(_coro);
        }

        return true;
    }
}}
