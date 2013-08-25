#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/multiWaiter.hpp"

#include <cassert>

namespace async { namespace impl
{
    Mutex::Mutex(bool recursive)
        : _recursive(recursive)
        , _owner()
        , _ownerDepth()
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
            _ownerDepth++;
            return true;
        }

        if(_recursive)
        {
            if(_owner == Coro::current())
            {
                _ownerDepth++;
                return true;
            }
        }

        return false;
    }

    bool Mutex::isLocked()
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _ownerDepth>0;
    }

    void Mutex::unlock()
    {
        std::unique_lock<std::mutex> l(_mtx);
        assert(_owner);
        assert(_ownerDepth);
        assert(_owner == Coro::current());

        _ownerDepth--;
        if(!_ownerDepth)
        {
            _owner = nullptr;

            //wakeup next if exists
            _owner = notifyOneAndGetCoro();
            if(_owner)
            {
                _ownerDepth++;
            }
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
                _ownerDepth++;
            }
            return false;
        }

        if(_recursive)
        {
            if(_owner == waiter->getCoro())
            {
                if(waiter->notify(this, true))
                {
                    _ownerDepth++;
                }
                return false;
            }
        }

        waiterAddInternal(waiter);
        return true;
    }

}}
