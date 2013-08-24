#ifndef _ASYNC_EVENT_HPP_
#define _ASYNC_EVENT_HPP_

#include "async/sizeofImpl.hpp"
#include "async/hiddenImpl.hpp"

namespace async
{
    namespace details
    {
        class MultiWaiter;
    }

    class Event
            : private HiddenImpl<impl::Event>
    {
        using Base = HiddenImpl<impl::Event>;

    private:
        Event(const Event &other) = delete;
        Event &operator=(const Event &other) = delete;

    public:
        Event(bool autoReset);
        ~Event();

        //void wait();
        size_t set();
        size_t pulse();

        bool isSet();
        bool reset();

    private:
        friend class details::MultiWaiter;
    };
}

#endif
