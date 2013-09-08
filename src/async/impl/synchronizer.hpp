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
