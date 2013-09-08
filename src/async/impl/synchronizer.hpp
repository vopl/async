#ifndef _ASYNC_IMPL_SYNCHRONIZER_HPP_
#define _ASYNC_IMPL_SYNCHRONIZER_HPP_

#include "async/impl/synchronizerWaiterNode.hpp"
#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class SynchronizerWaiterNode;

    ////////////////////////////////////////////////////////////////////////////////
    class Synchronizer
    {
    public:
        virtual bool tryAcquire() = 0;
        virtual bool waiterAdd(SynchronizerWaiterNode &node) = 0;
        virtual void waiterDel(SynchronizerWaiterNode &node) = 0;

    protected:
        Synchronizer();
        ~Synchronizer();

        bool empty();
        void enqueue(SynchronizerWaiterNode &node);
        SynchronizerWaiterNode *dequeue();
        void remove(SynchronizerWaiterNode &node);

    private:
        SynchronizerWaiterNode _leftBank;
        SynchronizerWaiterNode _rightBank;

    };
}}

#endif
