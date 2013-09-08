#include "async/impl/synchronizerWaiterNode.hpp"
#include "async/impl/event.hpp"
#include "async/impl/mutex.hpp"

namespace async { namespace impl
{
    SynchronizerWaiterNode::SynchronizerWaiterNode()
//        : _right()
//        , _left()
//        , _syncronizer()
//        , _waiter()
//        , _synchronizerIndex()
    {
    }

    bool SynchronizerWaiterNode::queued() const
    {
        if(_right)
        {
            assert(_left);
            return true;
        }
        assert(!_left);
        return false;
    }

    bool SynchronizerWaiterNode::tryAcquire()
    {
        if(est_event == _synchronizerType)
        {
            return _synchronizer._event->tryAcquire();
        }

        return _synchronizer._mutex->tryLock();
    }

    bool SynchronizerWaiterNode::waiterAdd(SynchronizerWaiterNode &node)
    {
        if(est_event == _synchronizerType)
        {
            return _synchronizer._event->waiterAdd(node);
        }

        return _synchronizer._mutex->waiterAdd(node);
    }

    void SynchronizerWaiterNode::waiterDel(SynchronizerWaiterNode &node)
    {
        if(est_event == _synchronizerType)
        {
            return _synchronizer._event->waiterDel(node);
        }

        return _synchronizer._mutex->waiterDel(node);
    }
}}

