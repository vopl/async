#ifndef _ASYNC_IMPL_MUTEX_HPP_
#define _ASYNC_IMPL_MUTEX_HPP_

#include "async/impl/synchronizer.hpp"
#include <atomic>
#include <vector>

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
        virtual bool waiterAdd(Waiter *waiter) override;
        virtual void waiterDel(Waiter *waiter) override;

    private:
        enum class State
                : uint32_t
        {
            unlocked,
            locked,
            busy

        };

        std::atomic<State> _state;

        typedef std::vector<Waiter *> TVWaiters;
        TVWaiters _waiters;

//Syncronizer
//        std::mutex _mtx;
//        std::deque<Waiter *> _waiters;

    };
}}

#endif
