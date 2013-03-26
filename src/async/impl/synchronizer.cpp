#include "async/impl/synchronizer.hpp"
#include "async/impl/anyWaiter.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>

namespace async { namespace impl
{
    Synchronizer::Synchronizer()
    {

    }

    Synchronizer::~Synchronizer()
    {
        //assert(_waiters.empty());
        for(const AnyWaiterPtr &waiter : _waiters)
        {
            bool b = waiter->notify(this);
            assert(!b);
        }
    }

    bool Synchronizer::waiterAdd(AnyWaiterPtr waiter)
    {
        std::lock_guard<std::mutex> l(_mtx);
        waiterAddInternal(waiter);
        return true;
    }

    void Synchronizer::waiterDel(AnyWaiterPtr waiter)
    {
        std::lock_guard<std::mutex> l(_mtx);

        std::deque<AnyWaiterPtr>::iterator iter = _waiters.begin();
        std::deque<AnyWaiterPtr>::iterator end = _waiters.end();
        for(; iter!=end; iter++)
        {
            if(waiter == *iter)
            {
                _waiters.erase(iter);
                return;
            }
        }
        //assert(!"already deleted?");
    }

    void Synchronizer::waiterAddInternal(AnyWaiterPtr waiter)
    {
        assert(!_mtx.try_lock() && "must be already locked");
        _waiters.push_back(waiter);
    }

    size_t Synchronizer::waitersAmount()
    {
        assert(!_mtx.try_lock() && "must be already locked");
        return _waiters.size();
    }

    size_t Synchronizer::notify(size_t waitersAmount)
    {
        assert(waitersAmount);
        assert(!_mtx.try_lock() && "must be already locked");

        std::deque<AnyWaiterPtr> waiters(_waiters);
        _mtx.unlock();

        size_t result(0);
        for(const AnyWaiterPtr &waiter : waiters)
        {
            if(waiter->notify(this))
            {
                result++;
                if(!--waitersAmount)
                {
                    return result;
                }
            }
        }

        return result;
    }

    CoroPtr Synchronizer::notifyOneAndGetCoro()
    {
        assert(!_mtx.try_lock() && "must be already locked");

        for(const AnyWaiterPtr &waiter : _waiters)
        {
            if(waiter->notify(this))
            {
                return waiter->getCoro();
            }
        }

        return CoroPtr();
    }

}}
