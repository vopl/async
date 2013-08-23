#ifndef _ASYNC_MULTIWAITER_HPP_
#define _ASYNC_MULTIWAITER_HPP_

#include "async/sizeofImpl.hpp"
#include "async/hiddenImpl.hpp"

#include <cstdint>

namespace async
{
    class Mutex;
    class Event;

    class MultiWaiter
            : public HiddenImpl<impl::MultiWaiter>
    {
        using Base = HiddenImpl<impl::MultiWaiter>;

    private:
        MultiWaiter(const MultiWaiter &other) = delete;
        MultiWaiter &operator=(const MultiWaiter &other) = delete;

    public:
        template <class... Waitable>
        MultiWaiter(Waitable &... waitables);
        ~MultiWaiter();

        uint32_t waitAny();
        void waitAll();

    private:
        void ctor(size_t waitablesAmount);
        void push(Mutex &mutex);
        void push(Event &event);

    private:
        template <class... Waitable>
        void collectWaitables(Mutex &mutex, Waitable &... waitables);

        template <class... Waitable>
        void collectWaitables(Event &event, Waitable &... waitables);

        void collectWaitables();
    };


    //////////////////////////////////////////////////
    template <class... Waitable>
    MultiWaiter::MultiWaiter(Waitable &... waitables)
    {
        ctor(sizeof...(waitables));
        collectWaitables(waitables...);
    }

    //////////////////////////////////////////////////
    inline void MultiWaiter::collectWaitables()
    {

    }

    //////////////////////////////////////////////////
    template <class... Waitable>
    void MultiWaiter::collectWaitables(Mutex &mutex, Waitable&... waitables)
    {
        push(mutex);
        collectWaitables(waitables...);
    }

    //////////////////////////////////////////////////
    template <class... Waitable>
    void MultiWaiter::collectWaitables(Event &event, Waitable&... waitables)
    {
        push(event);
        collectWaitables(waitables...);
    }

}

#endif
