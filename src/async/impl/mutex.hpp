#ifndef _ASYNC_IMPL_MUTEX_HPP_
#define _ASYNC_IMPL_MUTEX_HPP_

#include "async/impl/synchronizer.hpp"

namespace async { namespace impl
{
    class Mutex
            : public Synchronizer
    {
    private:
        Mutex(const Mutex &other) = delete;
        Mutex &operator=(const Mutex &other) = delete;

    public:
        Mutex();
        virtual ~Mutex();

        void lock();
        bool tryLock();

        bool isLocked();
        void unlock();

    private:
        virtual bool waiterAdd(MultiWaiter *waiter);

    private:
        Coro    *_owner;

//Syncronizer
//        std::mutex _mtx;
//        std::deque<MultiWaiter *> _waiters;

    };
}}

#endif
