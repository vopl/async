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
    MultiWaiter::MultiWaiter(uint32_t synchronizersAmount)
        : _synchronizersBuffer(
              (synchronizersAmount <= sizeof(_inlineSynchronizersBuffer)/sizeof(Synchronizer *)) ?
                  _inlineSynchronizersBuffer :
                  new Synchronizer * [synchronizersAmount])
        , _usedSynchronizersAmount(0)
        , _notified(markActive)
    {

    }

    MultiWaiter::~MultiWaiter()
    {
        if(_synchronizersBuffer != _inlineSynchronizersBuffer)
        {
            delete [] _synchronizersBuffer;
        }

    }


    void MultiWaiter::push(Synchronizer *synchronizer)
    {
        _synchronizersBuffer[_usedSynchronizersAmount++] = synchronizer;
    }

    uint32_t MultiWaiter::waitAny()
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        _coro = Coro::current()->shared_from_this();
        assert(_coro);

        for(uint32_t i(0); i<_usedSynchronizersAmount; i++)
        {
            if(!_synchronizersBuffer[i]->waiterAdd(this))
            {
                uint32_t notified= _notified.load();
                assert(_notified.load() <= i);

                for(uint32_t j(0); j<i; j++)
                {
                    _synchronizersBuffer[j]->waiterDel(this);
                }

                return i;
            }
        }

        //LOCK l.release();

        bool doLoop = true;
        while(doLoop)
        {
            uint32_t was = markActive;
            if(_notified.compare_exchange_weak(was, markDeactivating))
            {
                Scheduler *scheduler = _coro->scheduler();
                scheduler->contextDeactivate(_coro.get(), &_notified, markActive);
                break;
            }

            switch(was)
            {
            case markActive:
                //await more
                break;
            case markPreNotified:
                //await more
                std::this_thread::yield();
                break;
            case markDeactivating:
                //impossible
                abort();
                break;
            default:
                assert(was < 1024);
                doLoop = false;
                break;
            }
        }



        //LOCK std::lock_guard<std::mutex> l2(_mtx);
        assert(_notified.load() < _usedSynchronizersAmount);

//        for(uint32_t i(0); i<_notified; i++)
//        {
//            _synchronizersBuffer[i]->waiterDel(this);
//        }
//        for(uint32_t i(_notified+1); i<_usedSynchronizersAmount; i++)
//        {
//            _synchronizersBuffer[i]->waiterDel(this);
//        }
        for(uint32_t i(0); i<_usedSynchronizersAmount; i++)
        {
            _synchronizersBuffer[i]->waiterDel(this);
        }

        return _notified.load();
    }

    const CoroPtr &MultiWaiter::getCoro()
    {
        return _coro;
    }

    bool MultiWaiter::notify(Synchronizer *notifier)
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        for(;;)
        {
            uint32_t was = markActive;
            if(_notified.compare_exchange_weak(was, markPreNotified))
            {
                break;
            }

            switch(was)
            {
            case markActive:
                //await more, spurious fail in _notified.compare_exchange_weak
                break;
            case markPreNotified:
                return false;
            case markDeactivating:
                //await more, context deactivating in progress
                std::this_thread::yield();
                break;
            default:
                assert(was < 1024);
                return false;
            }
        }

        for(uint32_t i(0); i<_usedSynchronizersAmount; i++)
        {
            if(notifier == _synchronizersBuffer[i])
            {
                _notified.store(i);
            }
        }
        assert(_notified.load() < _usedSynchronizersAmount);


        assert(_coro);
        Scheduler *scheduler = _coro->scheduler();
        scheduler->coroReadyIfHolded(_coro.get());

        return true;
    }
}}
