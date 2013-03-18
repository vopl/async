#ifndef _ASYNC_IMPL_EVENT_HPP_
#define _ASYNC_IMPL_EVENT_HPP_

#include "async/event.hpp"
#include "async/impl/coro.hpp"

#include <memory>
#include <mutex>
#include <deque>

namespace async { namespace impl
{
    class Event
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
        std::mutex _mtx;
        ::async::Event::EResetMode _erm_;
        bool _state;

        std::deque<CoroPtr> _waiters;
    };
}}

#endif
