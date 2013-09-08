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
        virtual bool tryAcquire() override;
        virtual bool waiterAdd(SynchronizerWaiterNode &node) override;
        virtual void waiterDel(SynchronizerWaiterNode &node) override;

    private:
        enum class State
                : uint32_t
        {
            unlocked,
            locked,
            busy
        };

        std::atomic<State> _state;
    };
}}

#endif
