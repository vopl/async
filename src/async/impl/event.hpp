#ifndef _ASYNC_IMPL_EVENT_HPP_
#define _ASYNC_IMPL_EVENT_HPP_

#include "async/event.hpp"
#include "async/impl/synchronizer.hpp"

#include <memory>

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
        ~Event();

        size_t set();
        size_t pulse();

        bool isSet();
        bool reset();

    private:
        virtual bool waiterAdd(AnyWaiterPtr waiter);

    private:
        bool _autoReset;
        bool _state;
    };
}}

#endif
