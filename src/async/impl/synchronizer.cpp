#include "async/impl/synchronizer.hpp"
#include "async/impl/multiWaiter.hpp"
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
        for(MultiWaiter *waiter : _waiters)
        {
            bool b = waiter->notify(this);
            assert(!b);
        }
    }

    bool Synchronizer::waiterAdd(MultiWaiter *waiter)
    {
        std::lock_guard<std::mutex> l(_mtx);
        waiterAddInternal(waiter);
        return true;
    }

    void Synchronizer::waiterDel(MultiWaiter *waiter)
    {
        std::lock_guard<std::mutex> l(_mtx);

        std::deque<MultiWaiter *>::iterator iter = _waiters.begin();
        std::deque<MultiWaiter *>::iterator end = _waiters.end();
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

    void Synchronizer::waiterAddInternal(MultiWaiter *waiter)
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

        std::deque<MultiWaiter *> waiters(_waiters);
        _mtx.unlock();

        size_t result(0);
        for(MultiWaiter *waiter : waiters)
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

        for(MultiWaiter *waiter : _waiters)
        {
            if(waiter->notify(this))
            {
                return waiter->getCoro();
            }
        }

        return CoroPtr();
    }

}}
