#ifndef _ASYNC_IMPL_EVENT_HPP_
#define _ASYNC_IMPL_EVENT_HPP_

#include "async/impl/synchronizer.hpp"
#include <cassert>

namespace async { namespace impl
{
    class Event
            : public Synchronizer
    {
    private:
        Event(const Event &other);//not impl
        Event &operator=(const Event &other);//not impl

    public:
        Event(bool autoReset);
        virtual ~Event();

        void wait();

        size_t set();
        size_t pulse();

        bool isSet();
        bool reset();

    private:
        virtual bool waiterAdd(MultiWaiter *waiter) override;
        virtual void waiterDel(MultiWaiter *waiter) override {assert(0);}
        size_t waitersAmount() {assert(0);return 0;}
        bool notifyOne() {assert(0); return false;}
        size_t notifyAll() {assert(0); return 0;}
        void waiterAddInternal(...) {assert(0);}

    private:
        bool _autoReset;
        bool _state;

//Syncronizer
//        std::mutex _mtx;
//        std::deque<MultiWaiter *> _waiters;

    };
}}

#endif
