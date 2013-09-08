#ifndef _ASYNC_IMPL_EVENT_HPP_
#define _ASYNC_IMPL_EVENT_HPP_

#include "async/impl/synchronizer.hpp"
#include <atomic>
#include <vector>
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
        virtual bool tryAcquire() override;
        virtual bool waiterAdd(SynchronizerWaiterNode &node) override;
        virtual void waiterDel(SynchronizerWaiterNode &node) override;

    private:
        enum class State
        {
            nonsignalled,
            signalled,
            busy
        };

        bool _autoReset;
        std::atomic<State> _state;
    };
}}

#endif
