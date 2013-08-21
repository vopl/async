#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/anyWaiter.hpp"

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

    bool Mutex::tryLock()
    {
        std::lock_guard<std::mutex> l(_mtx);

        if(_ownerDepth)
        {
            return false;
        }

        _owner = Coro::current()->shared_from_this();
        _ownerDepth++;
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
        assert(_owner.get() == Coro::current());

        _ownerDepth--;
        if(!_ownerDepth)
        {
            _owner.reset();

            //wakeup next if exists
            _owner = notifyOneAndGetCoro();
            if(_owner)
            {
                _ownerDepth++;
            }
        }
    }

    bool Mutex::waiterAdd(AnyWaiterPtr waiter)
    {
        std::lock_guard<std::mutex> l(_mtx);
        if(!_owner)
        {
            if(waiter->notify(this))
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
                if(waiter->notify(this))
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
