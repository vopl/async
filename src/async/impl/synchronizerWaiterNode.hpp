#ifndef _ASYNC_IMPL_SYNCHRONIZERWAITERNODE_HPP_
#define _ASYNC_IMPL_SYNCHRONIZERWAITERNODE_HPP_

#include <memory>
#include <mutex>
#include <cassert>

namespace async { namespace impl
{
    class Synchronizer;
    class Waiter;

    ////////////////////////////////////////////////////////////////////////////////
    class SynchronizerWaiterNode
    {
    private:
        SynchronizerWaiterNode(const SynchronizerWaiterNode &) = delete;
        void operator=(const SynchronizerWaiterNode &) = delete;

    public:
        SynchronizerWaiterNode *_right;
        SynchronizerWaiterNode *_left;

        Synchronizer    *_syncronizer;
        Waiter          *_waiter;
        uint8_t         _synchronizerIndex;

        SynchronizerWaiterNode()
            : _right()
            , _left()
            , _syncronizer()
            , _waiter()
            , _synchronizerIndex()
        {
        }

        bool queued() const
        {
            if(_right)
            {
                assert(_left);
                return true;
            }
            assert(!_left);
            return false;
        }
    };
}}

#endif
