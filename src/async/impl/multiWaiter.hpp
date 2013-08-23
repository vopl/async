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
    typedef std::shared_ptr<Coro> CoroPtr;

    class MultiWaiter
    {
    public:
        MultiWaiter(uint32_t synchronizersAmount);
        ~MultiWaiter();

        void push(Synchronizer *synchronizer);

        uint32_t waitAny();
        void waitAll();

        const CoroPtr &getCoro();

    protected:
        //from Synchronizer
        friend class ::async::impl::Synchronizer;
        friend class ::async::impl::Event;
        friend class ::async::impl::Mutex;
        bool notify(Synchronizer *notifier);

    private:
        //std::mutex  _mtx;
//        std::deque<Synchronizer *> _synchronizers;
//        std::deque<Synchronizer *> _synchronizersInitial;

        Synchronizer *_inlineSynchronizersBuffer[8];
        Synchronizer **_synchronizersBuffer;
        uint32_t _usedSynchronizersAmount;

        std::atomic<uint32_t> _notified;
        static const uint32_t markActive = (uint32_t)-1;
        static const uint32_t markPreNotified = (uint32_t)-2;
        static const uint32_t markDeactivating = (uint32_t)-3;
        CoroPtr _coro;
    };

}}

#endif
