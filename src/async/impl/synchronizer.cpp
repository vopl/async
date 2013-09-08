#include "async/impl/synchronizer.hpp"
#include "async/impl/waiter.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>
#include <cstring>

namespace async { namespace impl
{
    Synchronizer::Synchronizer()
    {
        _leftBank._right = &_rightBank;
        _rightBank._left = &_leftBank;
    }

    Synchronizer::~Synchronizer()
    {
        assert(_leftBank._right == &_rightBank);
        assert(_rightBank._left == &_leftBank);
    }

    bool Synchronizer::empty()
    {
        return _leftBank._right == &_rightBank;
    }

    void Synchronizer::enqueue(SynchronizerWaiterNode &node)
    {
        assert(nullptr != node._synchronizer._event);
        assert(nullptr != node._waiter);

        node._left = &_leftBank;
        node._right = _leftBank._right;

        node._left->_right = &node;
        node._right->_left = &node;
    }

    SynchronizerWaiterNode *Synchronizer::dequeue()
    {
        SynchronizerWaiterNode *target = _rightBank._left;
        if(target == &_leftBank)
        {
            return nullptr;
        }

        target->_left->_right = target->_right;
        target->_right->_left = target->_left;

        target->_left = target->_right = nullptr;

        assert(nullptr != target->_synchronizer._event);
        assert(nullptr != target->_waiter);

        return target;
    }

    void Synchronizer::remove(SynchronizerWaiterNode &node)
    {
        node._left->_right = node._right;
        node._right->_left = node._left;

        node._left = node._right = nullptr;
        assert(nullptr != node._synchronizer._event);
        assert(nullptr != node._waiter);
    }
}}
