#include "async/impl/anyWaiter.hpp"
#include "async/event.hpp"
#include "async/impl/event.hpp"
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

//    void waiterPush(Waiter *waiter, const ::async::Mutex &waitable)
//    {
//        waiter->push(waitable);
//    }

    size_t waiterExec(AnyWaiterPtr waiter)
    {
        size_t result = waiter->exec();
        return result;
    }


    AnyWaiter::AnyWaiter()
        : _notified(-1)
    {

    }

    AnyWaiter::~AnyWaiter()
    {

    }

    void AnyWaiter::push(const ::async::Event &waitable)
    {
        _synchronizers.push_back(waitable._implEvent);
    }

//    void Waiter::push(const ::async::Mutex &waitable)
//    {
//    }

    size_t AnyWaiter::exec()
    {
        std::unique_lock<std::mutex> l(_mtx);

        for(size_t i(0); i<_synchronizers.size(); i++)
        {
            if(!_synchronizers[i]->waiterAdd(shared_from_this()))
            {
                _notified = i;

                std::deque<SynchronizerPtr> synchronizers;
                synchronizers.swap(_synchronizers);

                l.unlock();
                for(size_t iRollback(0); iRollback<i; iRollback++)
                {
                    synchronizers[iRollback]->waiterDel(shared_from_this());
                }
                return i;
            }
        }

        if(_notified != (size_t)-1)
        {
            return _notified;
        }

        _coro = Coro::current()->shared_from_this();
        assert(_coro);
        l.release();

        Scheduler *scheduler = _coro->scheduler();
        scheduler->contextDeactivate(_coro.get(), &_mtx);

        std::lock_guard<std::mutex> l2(_mtx);
        assert(_notified != (size_t)-1);
        return _notified;
    }

    bool AnyWaiter::notify(Synchronizer *notifier)
    {
        std::unique_lock<std::mutex> l(_mtx);

        if(_notified != (size_t)-1)
        {
            return false;
        }

        for(size_t i(0); i<_synchronizers.size(); i++)
        {
            if(notifier == _synchronizers[i].get())
            {
                _notified = i;
                assert(_coro);
                Scheduler *scheduler = _coro->scheduler();
                scheduler->coroReadyIfHolded(_coro.get());
                _coro.reset();
                break;
            }
        }

        assert(_notified != (size_t)-1 && "alien notifier?");
        if(_notified == (size_t)-1)
        {
            return false;
        }

        size_t notified = _notified;
        std::deque<SynchronizerPtr> synchronizers;
        synchronizers.swap(_synchronizers);
        l.unlock();

        //TODO classic deadlock
        // event1.set [e1] - waiter.notify[] - event2.delWaiter[e2]
        // event2.set [e2] - waiter.notify[] - event1.delWaiter[e1]
        for(size_t i(0); i<notified; i++)
        {
            synchronizers[i]->waiterDel(shared_from_this());
        }
        for(size_t i(notified+1); i<synchronizers.size(); i++)
        {
            synchronizers[i]->waiterDel(shared_from_this());
        }
        synchronizers.clear();

        return true;
    }

}}
