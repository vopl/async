#ifndef _ASYNC_IMPL_WAITER_HPP_
#define _ASYNC_IMPL_WAITER_HPP_

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
    class SynchronizerWaiterNode;
    class Mutex;
    class Event;

    class Coro;

    class Waiter
    {
    public:
        Waiter(impl::SynchronizerWaiterNode *synchronizerWaiterNodes);
        Waiter(impl::SynchronizerWaiterNode *synchronizerWaiterNodes, uint32_t synchronizerWaiterNodesAmount);
        ~Waiter();

        void push(Event *event);
        void push(Mutex *mutex);

        uint32_t any();
        void all();

        Coro *getCoro();

    protected:
        //from Synchronizer
        friend class ::async::impl::Synchronizer;
        friend class ::async::impl::Event;
        friend class ::async::impl::Mutex;
        bool notify(impl::SynchronizerWaiterNode &node, bool guaranteeNonInactive);

    private:
        std::atomic<uint32_t> _state;//mark or fired Synchronizer index
        static const uint32_t markActive = (uint32_t)-1;
        static const uint32_t markPreNotified = (uint32_t)-2;
        static const uint32_t markDeactivating = (uint32_t)-3;
        static const uint32_t markInactive = (uint32_t)-4;

        SynchronizerWaiterNode *_synchronizerWaiterNodes;
        uint32_t _synchronizerWaiterNodesAmount;

        Coro *_coro;
    };

}}

#endif
