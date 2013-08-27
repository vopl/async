#ifndef _ASYNC_IMPL_MULTIWAITER_HPP_
#define _ASYNC_IMPL_MULTIWAITER_HPP_

#include <memory>
#include <mutex>
#include <deque>
#include <atomic>
#include <cstdint>

namespace async
{
    class Event;
    class Mutex;
}

namespace async { namespace impl
{
    class Synchronizer;
    class Mutex;
    class Event;

    class Coro;

    class MultiWaiter
    {
    public:
        MultiWaiter(Synchronizer **synchronizersBuffer);
        ~MultiWaiter();

        void push(Synchronizer *synchronizer);

        uint32_t waitAny();
        void waitAll();

        Coro *getCoro();

    protected:
        //from Synchronizer
        friend class ::async::impl::Synchronizer;
        friend class ::async::impl::Event;
        friend class ::async::impl::Mutex;
        bool notify(Synchronizer *notifier, bool guaranteeNonInactive);

    private:
        std::atomic<uint32_t> _state;//mark or fired Synchronizer index
        static const uint32_t markActive = (uint32_t)-1;
        static const uint32_t markPreNotified = (uint32_t)-2;
        static const uint32_t markDeactivating = (uint32_t)-3;
        static const uint32_t markInactive = (uint32_t)-4;

        Synchronizer **_synchronizersBuffer;
        uint32_t _synchronizersAmount;

        Coro *_coro;
    };

}}

#endif
