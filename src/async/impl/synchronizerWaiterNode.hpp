#ifndef _ASYNC_IMPL_SYNCHRONIZERWAITERNODE_HPP_
#define _ASYNC_IMPL_SYNCHRONIZERWAITERNODE_HPP_

#include <memory>
#include <mutex>
#include <cassert>

namespace async { namespace impl
{
    class Synchronizer;
    class Waiter;

    class Event;
    class Mutex;

    ////////////////////////////////////////////////////////////////////////////////
    class SynchronizerWaiterNode
    {
    private:
        SynchronizerWaiterNode(const SynchronizerWaiterNode &) = delete;
        void operator=(const SynchronizerWaiterNode &) = delete;

    public:
        SynchronizerWaiterNode *_right;
        SynchronizerWaiterNode *_left;

        Waiter          *_waiter;

        union
        {
            Event *_event;
            Mutex *_mutex;
        } _synchronizer;

        enum ESynchronizerType : uint8_t
        {
            est_event,
            est_mutex,
        } _synchronizerType;

        uint8_t         _synchronizerIndex;

        SynchronizerWaiterNode();

        bool queued() const;

        bool tryAcquire();
        bool waiterAdd(SynchronizerWaiterNode &node);
        void waiterDel(SynchronizerWaiterNode &node);

    };
}}

#endif
