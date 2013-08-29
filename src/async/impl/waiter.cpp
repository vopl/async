#include "async/impl/waiter.hpp"
#include "async/event.hpp"
#include "async/mutex.hpp"
#include "async/impl/event.hpp"
#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>
#include <iostream>
#include <algorithm>

namespace async { namespace impl
{
    Waiter::Waiter(Synchronizer **synchronizersBuffer)
        : _state(markActive)
        , _synchronizersBuffer(synchronizersBuffer)
        , _synchronizersAmount(0)
        , _coro()
    {

    }

    Waiter::Waiter(Synchronizer **synchronizersBuffer, uint32_t synchronizersAmount)
        : _state(markActive)
        , _synchronizersBuffer(synchronizersBuffer)
        , _synchronizersAmount(synchronizersAmount)
        , _coro()
    {

    }

    Waiter::~Waiter()
    {
    }


    void Waiter::push(Synchronizer *synchronizer)
    {
        _synchronizersBuffer[_synchronizersAmount++] = synchronizer;
    }

    uint32_t Waiter::any()
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        _coro = Coro::current();
        assert(_coro);

        static __thread uint32_t seed = 0;
        seed = seed * 1103515245 + (uint32_t)(intptr_t)_coro;
        uint32_t offset = (seed>>16) % _synchronizersAmount;

        for(uint32_t idxTry(offset); idxTry<_synchronizersAmount; idxTry++)
        {
            if(_synchronizersBuffer[idxTry]->tryAcquire())
            {
                return idxTry;
            }
        }

        for(uint32_t idxTry(0); idxTry<offset; idxTry++)
        {
            if(_synchronizersBuffer[idxTry]->tryAcquire())
            {
                return idxTry;
            }
        }

        for(uint32_t idxAdd(0); idxAdd<_synchronizersAmount; idxAdd++)
        {
            if(!_synchronizersBuffer[idxAdd]->waiterAdd(this))
            {
                for(uint32_t idxDel(0); idxDel<idxAdd; idxDel++)
                {
                    _synchronizersBuffer[idxDel]->waiterDel(this);
                }

                uint32_t notified = _state.load();
                while(notified == markPreNotified)
                {
                    std::this_thread::yield();
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
                std::this_thread::yield();
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



        assert(_state.load() < _synchronizersAmount);

        for(uint32_t i(0); i<_synchronizersAmount; i++)
        {
            _synchronizersBuffer[i]->waiterDel(this);
        }

        return _state.load();
    }

    Coro *Waiter::getCoro()
    {
        return _coro;
    }

    bool Waiter::notify(Synchronizer *notifier, bool guaranteeNonInactive)
    {
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
                    std::this_thread::yield();
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

        for(uint32_t i(0); i<_synchronizersAmount; i++)
        {
            if(notifier == _synchronizersBuffer[i])
            {
                _state.store(i);
                break;
            }
        }

        assert(_coro);
        if(!coroActive)
        {
            Scheduler *scheduler = _coro->scheduler();
            scheduler->coroReady(_coro);
        }

        return true;
    }
}}
