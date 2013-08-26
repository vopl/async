#include "async/impl/mutex.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/multiWaiter.hpp"

#include <thread>
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
        Coro *was = nullptr;
        if(_owner.compare_exchange_strong(was, Coro::current()))
        {
            return true;
        }

        return false;
    }

    bool Mutex::isLocked()
    {
        return _owner.load(/*relaxed?*/) ? true : false;
    }

    Coro *markLocked = (Coro *)1;
    void Mutex::unlock()
    {
        for(;;)
        {
            Coro *was = Coro::current();

            if(_owner.compare_exchange_strong(was, markLocked))
            {
                //wakeup next if exists
                Coro *next = nullptr;
                while(!_waiters.empty())
                {
                    MultiWaiter *waiter = _waiters.front();
                    _waiters.erase(_waiters.begin());

                    next = waiter->getCoro();
                    if(waiter->notify(this, false))
                    {
                        break;
                    }

                    next = nullptr;
                }
                _owner.exchange(next);
                break;
            }

            if(Coro::current() == was)
            {
                //spurious failure, try one more
                continue;
            }
            if(markLocked == was)
            {
                //locked by some one, wait
                std::this_thread::yield();
                continue;
            }
        }
    }

    bool Mutex::waiterAdd(MultiWaiter *waiter)
    {
        Coro *was = nullptr;
        Coro *current = waiter->getCoro();

        for(;;)
        {
            if(_owner.compare_exchange_weak(was, current))
            {
                if(!waiter->notify(this, true))
                {
                    _owner.exchange(nullptr);
                }
                return false;
            }

            if(!was)
            {
                //spurious failure, try one more
                continue;
            }
            else if(markLocked == was)
            {
                //do spin, wait
                std::this_thread::yield();
                was = nullptr;
                continue;
            }

            //already locked by over waiter, put to waiters queue
            if(_owner.compare_exchange_strong(was, markLocked))
            {
                _waiters.push_back(waiter);

                assert(markLocked == _owner.load());
                _owner.exchange(was);
                return true;
            }
            else
            {
                //waiter is dequeued yet, try one more
                was = nullptr;
                continue;
            }
        }

        assert(!"never here");
        return false;
    }

    void Mutex::waiterDel(MultiWaiter *waiter)
    {
        Coro *was = nullptr;

        for(;;)
        {
            assert(nullptr == was);
            if(_owner.compare_exchange_weak(was,markLocked))
            {
                //stop loop, remove waiter
                break;
            }

            if(markLocked == was)
            {
                std::this_thread::yield();
                was = nullptr;
                continue;
            }

            //already locked by over waiter, do local lock from him
            if(_owner.compare_exchange_strong(was, markLocked))
            {
                //stop loop, remove waiter
                break;
            }
            else
            {
                //waiter is not queued yet, try one more
                was = nullptr;
                continue;
            }
        }

        for(std::vector<MultiWaiter *>::iterator iter(_waiters.begin()), end(_waiters.end()); iter != end; ++iter)
        {
            if(waiter == *iter)
            {
                _waiters.erase(iter);
                break;
            }
        }

        assert(markLocked == _owner.load());
        _owner.exchange(was);
    }


}}
