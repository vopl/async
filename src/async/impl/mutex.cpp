#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/multiWaiter.hpp"

#include <cassert>

namespace async { namespace impl
{
    Mutex::Mutex()
        : _owner()
    {
    }

    Mutex::~Mutex()
    {
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
        std::lock_guard<std::mutex> l(_mtx);
        if(!_owner)
        {
            _owner = Coro::current();
            return true;
        }

        return false;
    }

    bool Mutex::isLocked()
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _owner ? true : false;
    }

    void Mutex::unlock()
    {
        std::unique_lock<std::mutex> l(_mtx);
        assert(_owner);
        assert(_owner == Coro::current());

        if(_owner)
        {
            _owner = nullptr;

            //wakeup next if exists
            _owner = notifyOneAndGetCoro();
        }
    }

    bool Mutex::waiterAdd(MultiWaiter *waiter)
    {
        std::lock_guard<std::mutex> l(_mtx);
        if(!_owner)
        {
            if(waiter->notify(this, true))
            {
                _owner = waiter->getCoro();
            }
            return false;
        }

        waiterAddInternal(waiter);
        return true;
    }

}}
