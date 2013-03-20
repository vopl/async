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
        Event(::async::Event::EResetMode erm, bool initial);
        ~Event();

        void set(::async::Event::EResetMode erm);

        bool isSet();
        void reset();

        void wait();

    private:
        ::async::Event::EResetMode _erm;
        bool _state;
    };
}}

#endif
