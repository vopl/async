#ifndef _ASYNC_EVENT_HPP_
#define _ASYNC_EVENT_HPP_

#include <memory>

namespace async
{
    namespace impl
    {
        class Event;
        typedef std::shared_ptr<Event> EventPtr;

        class AnyWaiter;
    }

    class Event
    {
    public:
        Event(bool autoReset);
        Event(const Event &other);
        Event(Event &&other);
        ~Event();

        Event &operator=(const Event &other);
        Event &operator=(Event &&other);

        void swap(Event &other);

        //void wait();
        size_t set();
        size_t pulse();

        bool isSet();
        bool reset();

    private:
        friend class impl::AnyWaiter;
        impl::EventPtr _implEvent;
    };
}

#endif
