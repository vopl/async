#include "async/impl/anyWaiter.hpp"
#include "async/event.hpp"
#include "async/mutex.hpp"
#include "async/impl/event.hpp"
#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>

namespace async { namespace impl
{
    AnyWaiterPtr waiterAlloc()
    {
        return AnyWaiterPtr(new AnyWaiter);
    }

    void waiterPush(AnyWaiterPtr waiter, const ::async::Event &waitable)
    {
        waiter->push(waitable);
    }

    void waiterPush(AnyWaiterPtr waiter, const ::async::Mutex &waitable)
    {
        waiter->push(waitable);
    }

    size_t waiterExec(AnyWaiterPtr waiter)
    {
        size_t result = waiter->exec();
        return result;
    }


    AnyWaiter::AnyWaiter()
        : _notified(markActive)
    {

    }

    AnyWaiter::~AnyWaiter()
    {

    }

    void AnyWaiter::push(const ::async::Event &waitable)
    {
        _synchronizersInitial.push_back(waitable._implEvent);
    }

    void AnyWaiter::push(const ::async::Mutex &waitable)
    {
        //TEMPORARY DISABLE _synchronizersInitial.push_back(waitable._implMutex);
    }

    size_t AnyWaiter::exec()
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        _coro = Coro::current()->shared_from_this();
        assert(_coro);

        for(size_t i(0); i<_synchronizersInitial.size(); i++)
        {
            _synchronizers.push_back(_synchronizersInitial[i]);
            if(!_synchronizers[i]->waiterAdd(shared_from_this()))
            {
                assert(_notified.load() == i);

                for(size_t j(0); j<i; j++)
                {
                    _synchronizers[j]->waiterDel(shared_from_this());
                }
                _synchronizers.clear();

                return i;
            }
        }

        //LOCK l.release();

        bool doLoop = true;
        while(doLoop)
        {
            size_t was = markActive;
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
        assert(_notified.load() < _synchronizers.size());

        for(size_t i(0); i<_notified; i++)
        {
            _synchronizers[i]->waiterDel(shared_from_this());
        }
        for(size_t i(_notified+1); i<_synchronizers.size(); i++)
        {
            _synchronizers[i]->waiterDel(shared_from_this());
        }
        _synchronizers.clear();


        return _notified.load();
    }

    const CoroPtr &AnyWaiter::getCoro()
    {
        return _coro;
    }

    bool AnyWaiter::notify(Synchronizer *notifier)
    {
        //LOCK std::unique_lock<std::mutex> l(_mtx);

        for(;;)
        {
            size_t was = markActive;
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

        for(size_t i(0); i<_synchronizers.size(); i++)
        {
            if(notifier == _synchronizers[i].get())
            {
                _notified.store(i);
            }
        }
        assert(_notified.load() < _synchronizers.size());


        assert(_coro);
        Scheduler *scheduler = _coro->scheduler();
        scheduler->coroReadyIfHolded(_coro.get());

        return true;
    }
}}
