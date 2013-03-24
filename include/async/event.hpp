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
        enum EResetMode
        {
            erm_default,
            erm_immediately,
            erm_afterNotifyOne,
            erm_afterNotifyAll,
            erm_manual
        };

    public:
        Event(EResetMode erm = erm_afterNotifyOne, bool initial = false);
        Event(const Event &other);
        Event(Event &&other);
        ~Event();

        Event &operator=(const Event &other);
        Event &operator=(Event &&other);

        void swap(Event &other);

        void set(EResetMode erm = erm_default);

        bool isSet();
        void reset();

    private:
        friend class impl::AnyWaiter;
        impl::EventPtr _implEvent;
    };
}

#endif
